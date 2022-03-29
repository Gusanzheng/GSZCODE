#include <myhttp.h>
#include <fstream>

//����http��Ӧ��һЩ״̬��Ϣ
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file form this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the request file.\n";

locker m_lock;
map<string, string> users;
int http_conn::m_user_count = 0;
int http_conn::m_epollfd = -1;



//��ʼ������,�ⲿ���ó�ʼ���׽��ֵ�ַ
void http_conn::init(int sockfd, const sockaddr_in& addr, char* root)
{
    m_sockfd = sockfd;
    m_address = addr;
    http_utils.addfd(m_epollfd, sockfd, true);
    m_user_count++;
    doc_root = root;
    init();
}
//��ʼ���½��ܵ�����,check_stateĬ��Ϊ����������״̬
void http_conn::init()
{
    bytes_to_send = 0;
    bytes_have_send = 0;
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = GET;
    m_url = 0;
    m_version = 0;
    m_content_length = 0;
    m_host = 0;
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    cgi = 0;
    m_state = 0;
    timer_flag = 0;
    improv = 0;

    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    memset(m_real_file, '\0', FILENAME_LEN);
}
//�ر����ӣ��ر�һ�����ӣ��ͻ�������һ
void http_conn::close_conn(bool real_close)
{
    if (real_close && (m_sockfd != -1))
    {
        printf("close %d\n", m_sockfd);
        http_utils.removefd(m_epollfd, m_sockfd);
        m_sockfd = -1;
        m_user_count--;
    }
}
//LT����ģʽ�£�������
bool http_conn::read_once()
{
    if (m_read_idx >= READ_BUFFER_SIZE)
    {
        return false;
    }
    int bytes_read = 0;

    //LT��ȡ����
    bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
    m_read_idx += bytes_read;

    if (bytes_read <= 0)
    {
        return false;
    }

    return true;
}
//����HTTP��Ӧ��Web��
bool http_conn::write()
{
    int temp = 0;

    /*��Ҫ���͵����ݳ���Ϊ0����ʾ��Ӧ����Ϊ�գ�һ�㲻������������*/
    if (bytes_to_send == 0)
    {
        http_utils.modfd(m_epollfd, m_sockfd, EPOLLIN);
        init();
        return true;
    }

    while (1)
    {
        /*����Ӧ���ķ��͸������*/
        temp = writev(m_sockfd, m_iv, m_iv_count);

        if (temp < 0)
        {
            if (errno == EAGAIN)//���д�������Ѿ����ˣ���ȴ���һ��EPOLLOUT�¼�
            {
                http_utils.modfd(m_epollfd, m_sockfd, EPOLLOUT);
                return true;
            }
            unmap();
            return false;
        }

        bytes_have_send += temp;//�����Ѿ����͵��ֽ�
        bytes_to_send -= temp;//����

        //��һ��iovecͷ����Ϣ�������Ѿ����꣬���͵ڶ���iovec����
        if (bytes_have_send >= m_iv[0].iov_len)
        {
            //���ټ�������ͷ����Ϣ
            m_iv[0].iov_len = 0;
            m_iv[1].iov_base = m_file_address + (bytes_have_send - m_write_idx);
            m_iv[1].iov_len = bytes_to_send;
        }
        else//�������͵�һ��iovecͷ����Ϣ������
        {
            m_iv[0].iov_base = m_write_buf + bytes_have_send;
            m_iv[0].iov_len = m_iv[0].iov_len - bytes_have_send;
        }

        /*�ж������Ƿ��Ѿ�����*/
        if (bytes_to_send <= 0)
        {
            //HTTP���ͳɹ�������HTTP������connection�ֶ��ж��Ƿ������ر�����
            unmap();
            //��epoll��������EPOLLONESHOT�¼�
            http_utils.modfd(m_epollfd, m_sockfd, EPOLLIN);

            if (m_linger)//�����������Ϊ������
            {
                init();//���³�ʼ��HTTP����
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}
//��������
void http_conn::process()
{
    std::cout << "http_conn::process() is here" << std::endl;
    HTTP_CODE read_ret = process_read();
    if (read_ret == NO_REQUEST)
    {
        http_utils.modfd(m_epollfd, m_sockfd, EPOLLIN);
        return;
    }
    bool write_ret = process_write(read_ret);
    if (!write_ret)
    {
        close_conn();
    }
    http_utils.modfd(m_epollfd, m_sockfd, EPOLLOUT);
}


/*****************************************************************/
//���ĺ�����������������״̬��
http_conn::HTTP_CODE http_conn::process_read()
{
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char* text = 0;

    while ((m_check_state == CHECK_STATE_CONTENT && line_status == LINE_OK) || ((line_status = parse_line()) == LINE_OK))
    {
        text = get_line();
        m_start_line = m_checked_idx;
        switch (m_check_state)
        {
        case CHECK_STATE_REQUESTLINE:
        {
            ret = parse_request_line(text);
            if (ret == BAD_REQUEST)
                return BAD_REQUEST;
            break;
        }
        case CHECK_STATE_HEADER:
        {
            ret = parse_headers(text);
            if (ret == BAD_REQUEST)
                return BAD_REQUEST;
            else if (ret == GET_REQUEST)
            {
                return do_request();
            }
            break;
        }
        case CHECK_STATE_CONTENT:
        {
            ret = parse_content(text);
            if (ret == GET_REQUEST)
                return do_request();
            line_status = LINE_OPEN;
            break;
        }
        default:
            return INTERNAL_ERROR;
        }
    }
    return NO_REQUEST;
}
//����ֵΪ�еĶ�ȡ״̬����LINE_OK,LINE_BAD,LINE_OPEN
http_conn::LINE_STATUS http_conn::parse_line()
{
    char temp;
    for (; m_checked_idx < m_read_idx; ++m_checked_idx)
    {
        temp = m_read_buf[m_checked_idx];
        if (temp == '\r')
        {
            if ((m_checked_idx + 1) == m_read_idx)
                return LINE_OPEN;
            else if (m_read_buf[m_checked_idx + 1] == '\n')
            {
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if (temp == '\n')
        {
            if (m_checked_idx > 1 && m_read_buf[m_checked_idx - 1] == '\r')
            {
                m_read_buf[m_checked_idx - 1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}
//����http�����У�������󷽷���Ŀ��url��http�汾��
http_conn::HTTP_CODE http_conn::parse_request_line(char* text)
{
    m_url = strpbrk(text, " \t");
    if (!m_url)
    {
        return BAD_REQUEST;
    }
    *m_url++ = '\0';
    char* method = text;
    if (strcasecmp(method, "GET") == 0)
        m_method = GET;
    else if (strcasecmp(method, "POST") == 0)
    {
        m_method = POST;
        cgi = 1;
    }
    else
        return BAD_REQUEST;
    m_url += strspn(m_url, " \t");
    m_version = strpbrk(m_url, " \t");
    if (!m_version)
        return BAD_REQUEST;
    *m_version++ = '\0';
    m_version += strspn(m_version, " \t");
    if (strcasecmp(m_version, "HTTP/1.1") != 0)
        return BAD_REQUEST;
    if (strncasecmp(m_url, "http://", 7) == 0)
    {
        m_url += 7;
        m_url = strchr(m_url, '/');
    }

    if (strncasecmp(m_url, "https://", 8) == 0)
    {
        m_url += 8;
        m_url = strchr(m_url, '/');
    }

    if (!m_url || m_url[0] != '/')
        return BAD_REQUEST;
    //��urlΪ/ʱ����ʾ�жϽ���
    if (strlen(m_url) == 1)
        strcat(m_url, "picture.html");
    m_check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}
//����http�����һ��ͷ����Ϣ
http_conn::HTTP_CODE http_conn::parse_headers(char* text)
{
    if (text[0] == '\0')
    {
        if (m_content_length != 0)
        {
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    else if (strncasecmp(text, "Connection:", 11) == 0)
    {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0)
        {
            m_linger = true;
        }
    }
    else if (strncasecmp(text, "Content-length:", 15) == 0)
    {
        text += 15;
        text += strspn(text, " \t");
        m_content_length = atol(text);
    }
    else if (strncasecmp(text, "Host:", 5) == 0)
    {
        text += 5;
        text += strspn(text, " \t");
        m_host = text;
    }
    else
    {
        //LOG_INFO("oop!unknow header: %s", text);
        std::cout << "oop!unknow header: " << *text << std::endl;
    }
    return NO_REQUEST;
}
//�ж�http�����Ƿ���������
http_conn::HTTP_CODE http_conn::parse_content(char* text)
{
    if (m_read_idx >= (m_content_length + m_checked_idx))
    {
        text[m_content_length] = '\0';
        //POST���������Ϊ������û���������
        m_string = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}
//���ĺ�������Ӧhttp��������
http_conn::HTTP_CODE http_conn::do_request()
{
    strcpy(m_real_file, doc_root);//����վ��Ŀ¼doc_root��ֵ��m_real_file
    int len = strlen(doc_root);

    /*������Ͼ������ϣ������ǵ�¼��ע�ᣬֱ�ӽ�url����վĿ¼ƴ��
    **����������welcome���棬����������ϵ�һ��ͼƬ*/
    strncpy(m_real_file + len, m_url, FILENAME_LEN - len - 1);
    
    //ͨ��stat��ȡ������Դ�ļ���Ϣ���ɹ�����Ϣ���µ�m_file_stat�ṹ��
    //ʧ�ܷ���NO_RESOURCE״̬����ʾ��Դ������
    if (stat(m_real_file, &m_file_stat) < 0)
        return NO_RESOURCE;
    //�ж��ļ���Ȩ�ޣ��Ƿ�ɶ������ɶ��򷵻�FORBIDDEN_REQUEST״̬
    if (!(m_file_stat.st_mode & S_IROTH))
        return FORBIDDEN_REQUEST;
    //�ж��ļ����ͣ������Ŀ¼���򷵻�BAD_REQUEST����ʾ����������
    if (S_ISDIR(m_file_stat.st_mode))
        return BAD_REQUEST;
    //��ֻ����ʽ��ȡ�ļ���������ͨ��mmap�����ļ�ӳ�䵽�ڴ���
    int fd = open(m_real_file, O_RDONLY);
    m_file_address = (char*)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);//�����ļ����������˷Ѻ�ռ��
    
    return FILE_REQUEST;//��ʾ�����ļ����ڣ��ҿ��Է���
}


/******************************************************************/
/*���ĺ�����д����,����״̬��*/
bool http_conn::process_write(HTTP_CODE ret)
{
    switch (ret)
    {
    case INTERNAL_ERROR:
    {
        add_status_line(500, error_500_title);
        add_headers(strlen(error_500_form));
        if (!add_content(error_500_form))
            return false;
        break;
    }
    case BAD_REQUEST:
    {
        add_status_line(404, error_404_title);
        add_headers(strlen(error_404_form));
        if (!add_content(error_404_form))
            return false;
        break;
    }
    case FORBIDDEN_REQUEST:
    {
        add_status_line(403, error_403_title);
        add_headers(strlen(error_403_form));
        if (!add_content(error_403_form))
            return false;
        break;
    }
    case FILE_REQUEST:
    {
        add_status_line(200, ok_200_title);
        if (m_file_stat.st_size != 0)
        {
            add_headers(m_file_stat.st_size);
            m_iv[0].iov_base = m_write_buf;
            m_iv[0].iov_len = m_write_idx;
            m_iv[1].iov_base = m_file_address;
            m_iv[1].iov_len = m_file_stat.st_size;
            m_iv_count = 2;
            bytes_to_send = m_write_idx + m_file_stat.st_size;
            return true;
        }
        else
        {
            const char* ok_string = "<html><body></body></html>";
            add_headers(strlen(ok_string));
            if (!add_content(ok_string))
                return false;
        }
    }
    default:
        return false;
    }
    m_iv[0].iov_base = m_write_buf;
    m_iv[0].iov_len = m_write_idx;
    m_iv_count = 1;
    bytes_to_send = m_write_idx;
    return true;
}
void http_conn::unmap()
{
    if (m_file_address)
    {
        munmap(m_file_address, m_file_stat.st_size);
        m_file_address = 0;
    }
}
bool http_conn::add_response(const char* format, ...)
{
    if (m_write_idx >= WRITE_BUFFER_SIZE)
        return false;
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx))
    {
        va_end(arg_list);
        return false;
    }
    m_write_idx += len;
    va_end(arg_list);

    return true;
}
bool http_conn::add_status_line(int status, const char* title)
{
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}
bool http_conn::add_headers(int content_len)
{
    return add_content_length(content_len) && add_linger() &&
        add_blank_line();
}
bool http_conn::add_content_length(int content_len)
{
    return add_response("Content-Length:%d\r\n", content_len);
}
bool http_conn::add_content_type()
{
    return add_response("Content-Type:%s\r\n", "text/html");
}
bool http_conn::add_linger()
{
    return add_response("Connection:%s\r\n", (m_linger == true) ? "keep-alive" : "close");
}
bool http_conn::add_blank_line()
{
    return add_response("%s", "\r\n");
}
bool http_conn::add_content(const char* content)
{
    return add_response("%s", content);
}



