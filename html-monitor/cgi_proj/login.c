/** =====================================================
 * Copyright ? hk. 2022-2025. All rights reserved.
 * File name  : login.c
 * Author     : qidaink
 * Date       : 2022-09-11
 * Version    :
 * Description: ��¼ҳ���cgiԴ�ļ������ļ�δʹ��CGIC�⣩
 * Others     :
 * Log        :
 * ======================================================
 */

#include <stdio.h>
#include <stdlib.h> /* *getenv */
#include <string.h>

/* ��������ȫ�ֱ������ڴ洢�û��������� */
char name[64];
char pass[64];
/* �������� */
char *get_cgi_data(FILE *fp, char *requestmethod);/* ��ȡ��ҳ�ύ������ */
void unencode_for_name_pass(char *input);/* ���룬���ж��û��������� */

int main(int argc, char const *argv[])
{
	/* 0.������ر��� */
	char *input;
	char *req_method;
	/* 1.���߱���������html�﷨������ */
	printf("Content-type: text/html\n\n"); //
	printf("The following is query reuslt:<br><br>");
	/* 2.��ȡ���� */
	req_method = getenv("REQUEST_METHOD");
	input = get_cgi_data(stdin, req_method); /* ��ȡURL ��������� */
	/* 3.���룬���ж��û���������,�����ȷ����ת��ѡ��ֿ���棬������ʾ���� */
	unencode_for_name_pass(input);

	return 0;
}

/**
 * @Function           : get_cgi_data
 * @brief              : ��ȡ��ҳ�ύ������
 * @param fp           : FILE����
 * @param requestmethod: char *����
 * @return             : char *���ͣ���ȡ������ҵ������
 * @Description        :
 */
char *get_cgi_data(FILE *fp, char *requestmethod)
{
	/* 0.������ر��� */
	char *input;
	int len;
	int size = 1024;
	int i = 0;
	/* 1.��ͬ������ʽ��ȡ�����ݸ�ʽ��ͬ����Ҫ���� */
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
 * @brief      : ���룬���ж��û���������
 * @param input: char *���ͣ�Ҫ���������
 * @return     : none
 * @Description: 
 */
void unencode_for_name_pass(char *input)
{
	/* 0.������ر��� */
	int i = 0;
	int j = 0;
	int num[2] = {0}; /* �洢�����Ⱥŵ�λ�� */
	/* 1.��ȡ�����ݴ��� */
	/* 1.1��ȡ�����ݸ�ʽ����ȡ�Ⱥ�λ�� */
	/** ���ǻ�ȡ��input�ַ������������µ���ʽ
	 *  username="admin"&password="aaaaa"
	 *  ����"Username="��"&Password="���ǹ̶��ģ���"admin"��"aaaaa"���Ǳ仯�ģ�Ҳ������Ҫ��ȡ��.
	 *  ���ݸ�ʽȡ��������ط���
	 * <form onsubmit="return isValidate(myform)" action="cgi-bin/login.cgi" method="post">
     *    �û���: <input type="text" name="username" id="user"> 
	 *	  ����: <input type="pass" name="userpass" id="userpass">
     *          <input type="submit" value="��¼" id="button">
     * </form>
	 */
	printf("getdata is: %s<br> \n", input);
	for (i = 0, j = 0; i < (int)strlen(input); i++)
	{
		if (input[i] == '=')
		{
			num[j] = i;/* ����Ⱥŵ��±�λ�� */
			printf("%d<br>",num[j]);
			j++;
		}
	}
	/* 1.2��ȡ�û���
	 * ǰ��9���ַ���UserName=����"UserName="��"&"֮���������Ҫȡ�������û���
	 */
	for (i = num[0] + 1, j = 0; i < (int)strlen(input); i++)
	{
		if (input[i] == '&') /* ���� & ��ʱ��˵�������Ѿ���ȡ��� */
		{
			name[j] = '\0';
			break;
		}
		name[j++] = input[i];
	} 
	/* 1.3��ȡ����
	 * ǰ�����и��ַ� + "&Password="���ַ� + Username���ַ����ǲ���Ҫ�ģ���ʡ�Ե���������
	 */
	for (i = num[1] + 1, j = 0; i < (int)strlen(input); i++)
	{
		pass[j++] = input[i];
	}
	pass[j] = '\0';
	/* 1.4��ӡһ����ȡ�������� */
	printf("Your Username is %s<br> Your Password is %s<br> \n", name, pass);

	printf("Content-type: text/html\n\n"); //���߱���������html�﷨������
	if ((strcmp(name, "aaa") == 0) && (strcmp(pass, "123") == 0))
	{
		/* �Զ���ת�����ҳ�� */
		printf("<script language='javascript'>document.location = 'http://192.168.10.102/choose.html'</script>");
	}
	else
	{
		printf("�û������������<br><br>");
		// exit(-1);
	}
}