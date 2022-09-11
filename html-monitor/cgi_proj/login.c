/** =====================================================
 * Copyright ? hk. 2022-2025. All rights reserved.
 * File name  : login.c
 * Author     : qidaink
 * Date       : 2022-09-11
 * Version    :
 * Description: 登录页面的cgi源文件（此文件未使用CGIC库）
 * Others     :
 * Log        :
 * ======================================================
 */

#include <stdio.h>
#include <stdlib.h> /* *getenv */
#include <string.h>

/* 定义两个全局变量用于存储用户名和密码 */
char name[64];
char pass[64];
/* 函数声明 */
char *get_cgi_data(FILE *fp, char *requestmethod);/* 获取网页提交的数据 */
void unencode_for_name_pass(char *input);/* 解码，并判断用户名，密码 */

int main(int argc, char const *argv[])
{
	/* 0.定义相关变量 */
	char *input;
	char *req_method;
	/* 1.告诉编译器，用html语法来解析 */
	printf("Content-type: text/html\n\n"); //
	printf("The following is query reuslt:<br><br>");
	/* 2.获取数据 */
	req_method = getenv("REQUEST_METHOD");
	input = get_cgi_data(stdin, req_method); /* 获取URL 编码的数据 */
	/* 3.解码，并判断用户名，密码,如果正确，跳转至选择仓库界面，否则提示错误 */
	unencode_for_name_pass(input);

	return 0;
}

/**
 * @Function           : get_cgi_data
 * @brief              : 获取网页提交的数据
 * @param fp           : FILE类型
 * @param requestmethod: char *类型
 * @return             : char *类型，获取到的网业的数据
 * @Description        :
 */
char *get_cgi_data(FILE *fp, char *requestmethod)
{
	/* 0.定义相关变量 */
	char *input;
	int len;
	int size = 1024;
	int i = 0;
	/* 1.不同的请求方式获取的数据格式不同，需要处理 */
	if (!strcmp(requestmethod, "GET"))
	{
		input = getenv("QUERY_STRING");
		return input;
	}
	else if (!strcmp(requestmethod, "POST"))
	{
		len = atoi(getenv("CONTENT_LENGTH"));
		input = (char *)malloc(sizeof(char) * (size + 1));

		if (len == 0)
		{
			input[0] = '\0';
			return input;
		}

		while (1)
		{
			input[i] = (char)fgetc(fp);
			if (i == size)
			{
				input[i + 1] = '\0';
				return input;
			}
			--len;
			if (feof(fp) || (!(len)))
			{
				i++;
				input[i] = '\0';
				return input;
			}
			i++;
		}
	}
	return NULL;
}

/**
 * @Function   : unencode_for_name_pass
 * @brief      : 解码，并判断用户名，密码
 * @param input: char *类型，要解码的数据
 * @return     : none
 * @Description: 
 */
void unencode_for_name_pass(char *input)
{
	/* 0.定义相关变量 */
	int i = 0;
	int j = 0;
	int num[2] = {0}; /* 存储两个等号的位置 */
	/* 1.获取的数据处理 */
	/* 1.1获取的数据格式，提取等号位置 */
	/** 我们获取的input字符串可能像如下的形式
	 *  username="admin"&password="aaaaa"
	 *  其中"Username="和"&Password="都是固定的，而"admin"和"aaaaa"都是变化的，也是我们要获取的.
	 *  数据格式取决于这个地方：
	 * <form onsubmit="return isValidate(myform)" action="cgi-bin/login.cgi" method="post">
     *    用户名: <input type="text" name="username" id="user"> 
	 *	  密码: <input type="pass" name="userpass" id="userpass">
     *          <input type="submit" value="登录" id="button">
     * </form>
	 */
	printf("getdata is: %s<br> \n", input);
	for (i = 0, j = 0; i < (int)strlen(input); i++)
	{
		if (input[i] == '=')
		{
			num[j] = i;/* 保存等号的下标位置 */
			printf("%d<br>",num[j]);
			j++;
		}
	}
	/* 1.2提取用户名
	 * 前面9个字符是UserName=，在"UserName="和"&"之间的是我们要取出来的用户名
	 */
	for (i = num[0] + 1, j = 0; i < (int)strlen(input); i++)
	{
		if (input[i] == '&') /* 遇到 & 的时候说明名字已经提取完毕 */
		{
			name[j] = '\0';
			break;
		}
		name[j++] = input[i];
	} 
	/* 1.3提取密码
	 * 前面所有个字符 + "&Password="个字符 + Username的字符数是不需要的，故省略掉，不拷贝
	 */
	for (i = num[1] + 1, j = 0; i < (int)strlen(input); i++)
	{
		pass[j++] = input[i];
	}
	pass[j] = '\0';
	/* 1.4打印一下提取到的数据 */
	printf("Your Username is %s<br> Your Password is %s<br> \n", name, pass);

	printf("Content-type: text/html\n\n"); //告诉编译器，用html语法来解析
	if ((strcmp(name, "aaa") == 0) && (strcmp(pass, "123") == 0))
	{
		/* 自动跳转到这个页面 */
		printf("<script language='javascript'>document.location = 'http://192.168.10.102/choose.html'</script>");
	}
	else
	{
		printf("用户名或密码错误<br><br>");
		// exit(-1);
	}
}