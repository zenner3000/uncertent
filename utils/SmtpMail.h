#ifndef SMTPMAIL_H
#define SMTPMAIL_H

#pragma once

#include <string>
#include <vector>

//uncomment this will use NON-ssl
//#define SKIP_PEER_VERIFICATION
//#define SKIP_HOSTNAME_VERIFICATION

#include <string>
using namespace std;

class SmtpMail{
public:

	SmtpMail(const string& charset="utf-8"); // 也可以传入utf
    virtual ~SmtpMail();

	//设置stmp服务器、用户名、密码、端口（端口其实不用指定，libcurl默认25，但如果是smtps则默认是465）
	void SetSmtpServer(const string &username, const string& password, const string& servername, const string &port="25");
	//发送者姓名，可以不用

	void SetSendName(const string& sendname);

	//发送者邮箱
	void SetSendMail(const string& sendmail);

	//添加收件人
	void AddRecvMail(const string& recvmail);

	//设置主题
	void SetSubject(const string &subject);

	//设置正文内容
	void SetBodyContent(const string &content);

	//添加附件
	void AddAttachment(const string &filename);

	//发送邮件
	bool SendMail();
private:

	//回调函数，将MIME协议的拼接的字符串由libcurl发出
	static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *stream);

	//创建邮件MIME内容
	void CreatMessage();

	//获取文件类型
	int GetFileType(string const& stype);

	//设置文件名
	void SetFileName(const string& FileName);

	//设置文件的contenttype
	void SetContentType(string const& stype);

	//得到文件名
	void GetFileName(const string& file, string& filename);

	//得到文件类型
	void GetFileType(const string& file, string& stype);

private:
	string m_strCharset; //邮件编码
	string m_strSubject; //邮件主题
	string m_strContent; //邮件内容
	string m_strFileName; //文件名
	string m_strMessage;// 整个MIME协议字符串
	string m_strUserName;//用户名
	string m_strPassword;//密码
	string m_strServerName;//smtp服务器
	string m_strPort;//端口
	string m_strSendName;//发送者姓名
	string m_strSendMail;//发送者邮箱
	string m_strContentType;//附件contenttype
	string m_strFileContent;//附件内容

	vector<string> m_vRecvMail; //收件人容器
	vector<string> m_vAttachMent;//附件容器
};


#endif // SMTPMAIL_H
