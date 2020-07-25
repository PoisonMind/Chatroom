#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sqlite3.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>


class MsgData
{
public:
	int work;					//cli工作指令
	int flag;					//回执flag
	char mess[1024];			//消息
	char account[30];			//用户登入账号
	char name[30];				//用户昵称
	char pass[30];				//用户密码
	char online[30];			//在线情况
	char time[30];				//获取时间
	int root;					//root权限	
	int jinyan;					//禁言flag
	char toName[30];			//发给谁
	char fromName[30];			//谁发的
	char fileName[30];			//文件
	char question[256];		    //密保问题
	char answer[256];			//密保答案
};

class chatFile
{
public:
	char mess[1024];
	char fromName[30];
	char toName[30];
	char time[30];
	int flag;				    //判断私聊还是群聊 1私聊，0群聊
};

char IP[15];				    //服务器的IP
short PORT = 7777;				//服务器服务端口
int clientSocket;
char myName[30];                //昵称
char myAccount[30];				//ID
int isChatOneOnline;			//判断是否在线
int slientFlag;					//禁言后不保存本地聊天记录
int rootFlag;					//判断管理员flag
int bossFlag;					//判断用户是否要退出到主程序
int noOneSlientFlag;			//判断是否有人被禁言，有=1


void registerNewAccount();//注册
void saveGroupChat(MsgData *msg,int flag);
void chatAll();
char *getTime();
void firstMenu();
void chatManual();//帮助手册
void chatOne();
void lookOnlinePeople();
void isRoot();
void makeSlient();
void releaseSlient();
void thirdMenu();
void setRoot();
void removeRoot();
void getOut();
void rootActions();
void sendFile();
void viewLocalChat();
void linkOffline();
void secondMenuAndAction();
void anyKeyToNext();
int enterAccount();
char *getTime();
void* recvThread(void* _clientSocket);
void updateName();
void updatePassword();
void retrieveAccountPassword();
void actions();
void lookSlientPeople();
void saveFile(MsgData *msg);


void* recvThread(void* _clientSocket)				//一直在这里接收
{
	int clientSocket = *(int*)_clientSocket;
	MsgData msg;
	while(1)
	{
		if(recv(clientSocket,&msg,sizeof(MsgData),0) <= 0)//每次都判断
		{
			printf("服务器断开链接\n");
			exit(-1);
		}
		switch(msg.work)
		{
			case 3:
				printf("（群聊）%s : %s\n",msg.name,msg.mess);
				saveGroupChat(&msg,1);
				break;	
			case 4:
				isChatOneOnline = 1;//没找到私聊的人或已下下线
				break;
			case 5: 
				if(isChatOneOnline != 1)
				{
					printf("（私聊）%s偷偷对你说：%s\n",msg.fromName,msg.mess);
					saveGroupChat(&msg,0);
				}
				break;
			case 6:  //在线人员
				printf("%s\n",msg.name);
				break;			
			case 7: 
				if(msg.flag == 1)
				{
					rootFlag = 1;
					printf("您是root用户请稍后\n");
				}
				else
				{
					printf("您不是root用户\n正在返回群聊........\n");
				}
				break;
			case 8:
				slientFlag = 1;
				printf("您已经被禁言了\n");
				break;
			case 9:
				printf("操作失败（对方可能不存在）请重新选择操作\n");
				break;
			case 10:
				printf("禁言 %s 成功\n",msg.toName);
				break;
			case 11:
				printf("解除 %s 的禁言成功\n",msg.toName);
				break;
			case 12:
				printf("文件发送成功\n正在返回群聊........\n");
				break;
			case 13:
				printf("文件发送失败（此人不群聊中不能接收文件）\n正在返回群聊........\n");
				break;
			case 15:
				saveFile(&msg);
				printf("从%s处接收到一个文件\n",msg.fromName);
				break;
			case 16:
				bossFlag = 1;  //退出到主程序
				break;
			case -1:
				printf("一些意想不到的错误发生了\n");
				break;
			case 18:
				printf("设置成功\n");
				break;
			case 19:
				slientFlag = 0;
				printf("您已被解除禁言\n");
				break;
			case 20:
				printf("%s\n",msg.mess);
				exit(0);
				break;
			case 21:
				printf("对方不存在或不在线\n");
				break;
			case 22://被禁言人员
				printf("%s\n",msg.name);
				break;
			case 23:
				noOneSlientFlag = 1;
				break;
			case 24:
				printf("你被设为管理员\n");
				break;
			case 25:
				printf("你被移除管理员\n");
				break;
		}
		if(bossFlag == 1)
		{
			break;
		}
	}
}

int ifIpError(char *ip)
{
	int i = 0;
	int j = 0;
	int flagi = 0;
	int last = 0;								//ip每一段的初始位置下标
	char temp[50];								//存放ip的每段
	int ipTemp = 0;								//每段ip转换为int型用来判断
	int tempi = 0;
	int num = 0;
	while(ip[i] != 0)
	{
		if(ip[i] == ' ')							//输入空格的错误
		{
			return 1;
		}
		if(ip[i] == '.' || ip[i + 1] == 0)
		{
			flagi = i;
			if(ip[i + 1] == 0)					//能够判断最后一段
			{
				flagi++;
			}	
			num++;								//计算.的数量，正确的应有3
			for(j = last; j < flagi; j++)
			{
				temp[tempi++] = ip[j];			//每段复制给temp
			}
			temp[tempi] = 0;
			last = flagi + 1;						//保存每次.后ip段的开始位置
			tempi = 0;
			ipTemp = atoi(temp);					//temp转为int
			if(!(ipTemp >= 0 && ipTemp <= 255))
			{
				return 1;						//超出范围直接返回
			}			
		}		
		i++;
	}
	if(num != 4)
	{
		return 1;
	}
	return 0;
}

void firstMenu()
{
	printf("\n");
	printf("\t聊天室demo\t\n");
	printf("\t*****************\n");
	printf("\t1.注册\n");
	printf("\t2.登入\n");
	printf("\t3.修改密码\n");
	printf("\t4.修改昵称\n");
	printf("\t5.找回密码\n");
	printf("\t6.退出\n");
	printf("\t群主需抢先注册昵称为root\n");
	printf("\t*****************\n");
}

void chatManual()  //群聊手册
{
	system("clear");
	printf("********************************************\n");
	printf("\t群聊操作指南\n");
	printf("q2：进入私聊模式（私聊模式下输back返回群聊）\n");
	printf("q3：查看在线人员\n");
	printf("q4：查看本地聊天记录\n");
	printf("q5：进入管理员操作\n");
	printf("q6：发送文件\n");
	printf("群聊模式下输入bye退出群聊返回主程序\n");
	printf("********************************************\n");
}

void chatOne()		//私聊
{
	MsgData msg;	
	char tempMess[1024];
	char oneName[30];
	lookOnlinePeople();
	sleep(1);
	while(1)
	{
		isChatOneOnline = 0;		//每次while将isChatOneOnline重置为0；
		msg.work = 4;
		printf("输入你想要私聊的人：(back退出私聊返群聊)\n");
		gets(oneName);
		if(strcmp(oneName,"back") == 0)
		{
			printf("正在返回群聊........\n");		
			return;
		}
		strcpy(msg.toName,oneName);
		strcpy(msg.fromName,myName);
		if(strcmp(msg.toName, msg.fromName) == 0)
		{
			printf("请勿和自己私聊！！\n");
			sleep(1);
			continue;
		}
		printf("检验中请等待........\n");
		send(clientSocket,&msg,sizeof(MsgData),0);
		sleep(1);		
		if(isChatOneOnline == 1)		//isChatOneOnline：若无此人就不开始私聊
		{
			printf("在线中没有找到此人\n");		
		}
		else
		{
			printf("您可以开始私聊了\n");
			break;		
		}
		memset(oneName,0,strlen(oneName));
	}
	msg.work = 5;//私聊用5
	while(1)
	{
		gets(tempMess);
		strcpy(msg.mess,tempMess);
		strcpy(msg.time,getTime());
		if(strcmp(tempMess,"back") == 0)
		{
			printf("正在返回群聊........\n");		
			return;
		}
		else
		{
			if(isChatOneOnline == 1)
			{
				printf("对方已不在群聊\n");
				break;
			}
			printf("你对 %s 偷偷说了：%s\n",msg.toName,msg.mess);		
			send(clientSocket,&msg,sizeof(MsgData),0);
		}
		memset(tempMess,0,sizeof(tempMess));
	}
}

void lookOnlinePeople()							//查看在群聊中的人员
{
	MsgData msg;
	msg.work = 6;
	printf("在线名单如下\n");
	send(clientSocket,&msg,sizeof(MsgData),0);	
}

void isRoot()							//root权限判断
{
	MsgData msg;
	msg.work = 7;
	strcpy(msg.account,myAccount);	
	send(clientSocket,&msg,sizeof(MsgData),0);		
}

void makeSlient()
{	
	sleep(1);				//因为send交给线程处理后，这边会继续往下走；所以要sleep
	MsgData msg;			//等线程那儿处理完；
	msg.work = 8;
	char tempName[30];
	printf("输入禁言姓名：");
	gets(tempName);
	strcpy(msg.toName,tempName);
	send(clientSocket,&msg,sizeof(MsgData),0);
}

void releaseSlient()
{
	MsgData msg;
	msg.work = 10;
	char tempName[30];	
	printf("输入解禁姓名：");
	gets(tempName);
	strcpy(msg.toName,tempName);
	send(clientSocket,&msg,sizeof(MsgData),0);
}

void thirdMenu()
{
	printf("	  root用户操作        \n");
	printf("\n");
	printf("1,禁言		2,解除禁言\n");
	printf("3,设置管理员	4,移除管理员\n");
	printf("5,踢人		6,返回群聊\n");
	printf("\n");
}

void setRoot()//设置管理员权限( 群主权限)
{
	MsgData msg;
	msg.work = 17;
	printf("输入你想要设置管理的人的昵称：");
	gets(msg.name);
	send(clientSocket,&msg,sizeof(MsgData),0);	
}

void removeRoot()
{
	MsgData msg;
	msg.work = 18;
	printf("输入你想要移除管理的人的昵称：");
	gets(msg.name);
	send(clientSocket,&msg,sizeof(MsgData),0);	
}

void getOut()
{
	MsgData msg;
	msg.work = 21;	
	printf("输入姓名：");
	gets(msg.toName);
	send(clientSocket,&msg,sizeof(MsgData),0);	
}

void lookSlientPeople()//查看被禁言人员
{
	MsgData msg;
	msg.work = 22;
	send(clientSocket,&msg,sizeof(MsgData),0);	
	printf("被禁言人员如下:\n");
}

void rootActions()
{
	char getWork[5];
	int actions;
	int flag = 0;
	while(1)
	{
		thirdMenu();
		printf("输入操作:");
		gets(getWork);
		actions = atoi(getWork);
		switch(actions)
		{
			case 1:
				lookOnlinePeople();
				printf("等待...\n");
				sleep(1);
				makeSlient();
				sleep(1);
				break;
			case 2:
				lookSlientPeople();
				noOneSlientFlag = 0;			
				sleep(1);
				if(noOneSlientFlag == 1)
				{
					printf("目前没有人被禁言\n");
					break;
				}
				releaseSlient();
				sleep(1);
				break;
			case 3:
				lookOnlinePeople();
				printf("等待...\n");
				sleep(1);
				setRoot();
				sleep(1);
				break;
			case 4:
				lookOnlinePeople();
				printf("等待...\n");
				sleep(1);
				removeRoot();
				sleep(1);
				break;
			case 5:
				lookOnlinePeople();
				printf("等待...\n");
				sleep(1);
				getOut();
				sleep(1);
				break;
			case 6:
				flag = 1;
				printf("正在返回群聊\n");
				break;
		}
		if(flag == 1)
		{
			break;
		}
	}
}

void sendFile()
{
	MsgData msg;
	msg.work = 13;
	char buf[1024] = {0};
	char tempName[30];
	char txt[30];
	printf("请输入要发送的对象昵称\n");
	gets(tempName);
	strcpy(msg.toName,tempName);
	printf("输入要传的文件(加后缀)\n");
	gets(txt);
	strcpy(msg.fileName,txt);
	sprintf(buf,"%s",txt);
	int pd = open(buf, O_RDONLY|O_CREAT);
	if (pd == -1)
	{
		perror ("error");
		return;
	}	
	int ret = 0;	
	memset(buf,0,sizeof(buf));	
	ret = read(pd, buf, 1024);
	buf[ret] = '\0';
	strcpy(msg.mess,buf);	
	strcpy(msg.fromName,myName);
	send(clientSocket,&msg,sizeof(MsgData),0);	
	close(pd);	
}

void saveGroupChat(MsgData *msg,int flag)		//保存聊天记录分俩种，群聊私聊分开（群聊）
{
	FILE *fp = fopen("localChat.txt","a+");
	chatFile temp;
	if(flag == 1)
	{
		strcpy(temp.toName,"All");
		strcpy(temp.fromName,msg->name);
	}
	else
	{
		strcpy(temp.toName,"you");
		strcpy(temp.fromName,msg->fromName);
	}
	strcpy(temp.mess,msg->mess);
	strcpy(temp.time,getTime());
	fwrite(&temp,sizeof(chatFile),1,fp);//将结构体直接写入文件 有bug。不管了

	fclose(fp);
}

void viewLocalChat()				//保存聊天记录分俩种，群聊私聊分开（私聊）
{
	FILE *fp = fopen("localChat.txt","r");
	chatFile temp;
	int ret = fread(&temp,sizeof(chatFile),1,fp);
	while(ret > 0)
	{
		printf("%s:",temp.time);
		printf("\t%s:",temp.fromName);
		printf(" 对%s:",temp.toName);
		printf("说：%s\n",temp.mess);	
		ret = fread(&temp,sizeof(chatFile),1,fp);
	}
	fclose(fp);
}

void saveFile(MsgData *msg)
{
	char buf[1024] = {0};
	sprintf(buf,"%s",msg->fileName);
	int pd = open(buf, O_WRONLY|O_CREAT);		
	if (pd == -1)
	{
		perror ("error");
		return;
	}
	memset(buf,0,sizeof(buf));
	strcpy(buf, msg->mess);
	write(pd,buf,strlen(buf));
	close(pd);	
}

void chatAll()				//群聊功能
{
	MsgData msg;	
	char tempMess[1024];	
	msg.work = 3;
	msg.flag = 1;
	strcpy(msg.name,myName);
	system("clear");
	printf("群聊\n");
	sprintf(msg.mess,"进入了群聊");	
	send(clientSocket,&msg,sizeof(MsgData),0);	
	memset(msg.mess,0,strlen(msg.mess));		
	while(1)
	{
		msg.flag = 0;
		gets(tempMess);		
		strcpy(msg.mess,tempMess);		
		strcpy(msg.time,getTime());	
		if (strcmp(msg.mess,"bye") == 0)					//输入 bye 表示退出群聊
		{
			memset(tempMess,0,sizeof(tempMess));		
			sprintf(msg.mess,"退出了群聊\n");
			send(clientSocket,&msg,sizeof(MsgData),0);		//将退出的消息发给公屏
			memset(msg.mess,0,strlen(msg.mess));	
			strcpy(msg.mess,"bye");						
			send(clientSocket,&msg,sizeof(MsgData),0);
			break;
		}
		else if(strcmp(msg.mess,"q2") == 0)				//q2进入私聊
		{
			chatOne();	
			sleep(1);
			printf("您已返回群聊\n");			
		}
		else if(strcmp(msg.mess,"q3") == 0)				//查看在线人员
		{
			lookOnlinePeople();
			sleep(1);
			printf("\n您已返回群聊\n");	
		}
		else if(strcmp(msg.mess,"q4") == 0)				//查看聊天记录
		{
			viewLocalChat();								

			sleep(1);
			printf("\n您已返回群聊\n");	
		}
		else if(strcmp(msg.mess,"q5") == 0)				//管理员操作
		{
			isRoot();    //root权限判断	
			sleep(1);
			if(rootFlag == 1)  //判断是否为root用户
			{
				rootActions();
			}
			printf("您已返回群聊\n");	
		}
		else if(strcmp(msg.mess,"q6") == 0)			//发送文件
		{
			sendFile();
			sleep(1);
			printf("您已返回群聊\n");
		}
		else if(strcmp(msg.mess,"help") == 0)	
		{
			chatManual();							//手册
			sleep(1);
			printf("您已返回群聊\n");
		}			
		else
		{
			send(clientSocket,&msg,sizeof(MsgData),0);
			sleep(1);
		}
		memset(msg.mess,0,strlen(msg.mess));	
		memset(msg.time,0,strlen(msg.time));	
		memset(tempMess,0,sizeof(tempMess));	
	}	
}

void linkOffline()
{
	MsgData msg;	
	msg.work = 15;	
	strcpy(msg.name,myName);	
	send(clientSocket,&msg,sizeof(MsgData),0);	
}

void secondMenuAndAction()
{
	char getWork[5];
	int actions;
	int flag = 0;
	while(1)
	{	
		sleep(1);
		printf("\t****************\n");
		printf("\t3.进入群聊\n");
		printf("\t6.离线消息接收\n");
		printf("\t5.退出到主程序\n");
		printf("\t****************\n");

		gets(getWork);
		actions = atoi(getWork);

		switch(actions)
		{
			case 3:printf("actions = %d\n",actions);chatAll();break;
			case 5:linkOffline();flag = 1;break;
		}

		if(flag == 1)
		{		
			bossFlag = 1;  //想要退出到主程序，那么将这个设置为1
			MsgData msg;
			msg.work = 16;
			send(clientSocket,&msg,sizeof(MsgData),0);
			break;
		}
	}
}

void anyKeyToNext()
{
	char str[50];
	printf("按任意键返回\n");
	gets(str);
	return;
}

void registerNewAccount()				//注册
{
	MsgData msg;
	msg.work = 1;
	int accountTemp;	
	char tempName[30];
	char tempAccount[30];
	char tempPassword[30];	
	srand((unsigned) time(NULL));				//时间函数生成随机
	accountTemp = rand()%10000;
	sprintf(msg.account,"%d",accountTemp);				//整形转换字符串的??种方法	
	printf("若不想注册，任一输入下输入out即可返回\n");
	printf("输入你的昵称：");
	gets(tempName);
	if(strcmp(tempName,"out") == 0)
	{
		return ;
	}
	printf("输入你的密码：");
	gets(tempPassword);
	if(strcmp(tempPassword,"out") == 0)
	{
		return ;
	}
	printf("请输入密保问题: ");
	gets(msg.question);
	if(strcmp(msg.question,"out") == 0)
	{
		return ;
	}
	printf("请输入密保答案: ");
	gets(msg.answer);	
	if(strcmp(msg.answer,"out") == 0)
	{
		return ;
	}
	strcpy(msg.name,tempName);
	strcpy(msg.pass,tempPassword);	
	if(strcmp(msg.name,"root") == 0)			
	{
		msg.root = 1;
		strcpy(msg.account,"111");
	}
	else
	{
		msg.root = 0;
	}
	send(clientSocket,&msg,sizeof(MsgData),0);		//发送给总控制台;	
	if(recv(clientSocket,&msg,sizeof(MsgData),0) <= 0)//每次都判断
	{
		printf("服务器断开链接\n");
		exit(-1);
	}		
	if(msg.flag == 1)
	{
		printf("注册成功\n");
		printf("\n您的登入账号为：%s 请务必牢记\n",msg.account);
	}
	else if(msg.flag == 0)
	{
		printf("服务器错误注册失败\n");
	}
	else if(msg.flag == 3)
	{
		printf("昵称重复请重新注册\n");
	}
} 

int enterAccount()									//登入
{
	MsgData msg;	
	msg.work = 2;	
	char tempAccount[30];
	char tempPassword[30];	//这块一点小bug，万一用户密码超过30，算了，不管了
	printf("输入你的账号：");
	gets(tempAccount);
	printf("输入你的密码：");
	gets(tempPassword);
	strcpy(msg.account,tempAccount);
	strcpy(msg.pass,tempPassword);	
	send(clientSocket,&msg,sizeof(MsgData),0);	
	if(recv(clientSocket,&msg,sizeof(MsgData),0) <= 0) //每次都判断
	{
		printf("服务器断开链接\n");
		exit(-1);
	}	
	if(msg.flag == 1)
	{
		printf("\n");
		printf("欢迎你%s\n",msg.name);
		printf("\n");
		strcpy(myName,msg.name);	
		strcpy(myAccount,msg.account);
	}
	else if(msg.flag == 2)
	{
		printf("该账号已登入\n");
		return 0;
	}
	else if(msg.flag == 3)
	{
		printf("密码错误\n");
		return 0;
	}
	else if(msg.flag == 5)
	{
		printf("无此用户\n");
		return 0;
	}
	else
	{
		printf("系统错误,请重试\n");
		return 0;
	}
	return 1;
}

char *getTime()							//获取时间函数
{
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	return asctime(timeinfo);
}

void updateName()
{
	MsgData msg;
	msg.work = 12;	
	char tempAccount[30];
	char tempPassword[30];
	char tempName[30];
	printf("验证账号和密码\n");
	printf("输入你的账号：");
	gets(tempAccount);
	printf("输入你的密码：");
	gets(tempPassword);
	strcpy(msg.account,tempAccount);
	strcpy(msg.pass,tempPassword);	
	send(clientSocket,&msg,sizeof(MsgData),0);	
	if(recv(clientSocket,&msg,sizeof(MsgData),0) <= 0)//每次都判断
	{
		printf("服务器断开链接\n");
		exit(-1);
	}	
	if(msg.flag == 0)
	{
		printf("账号或密码错误\n");
		msg.flag = 2;	
		send(clientSocket,&msg,sizeof(MsgData),0);
		return;
	}
	else if(msg.flag == 1)
	{
		printf("账号密码正确\n");
		printf("输入想要修改成的昵称:");
		gets(tempName);
		strcpy(msg.name,tempName);
		msg.flag = 3;	
		send(clientSocket,&msg,sizeof(MsgData),0);
		printf("操作完成\n");
	}
}

void updatePassword()
{
	MsgData msg;	
	msg.work = 11;	
	char tempAccount[30];
	char tempPassword[30];
	printf("验证账号和密码\n");
	printf("输入你的账号：");
	gets(tempAccount);
	printf("输入你的密码：");
	gets(tempPassword);
	strcpy(msg.account,tempAccount);
	strcpy(msg.pass,tempPassword);	
	send(clientSocket,&msg,sizeof(MsgData),0);	
	if(recv(clientSocket,&msg,sizeof(MsgData),0) <= 0)//每次都判断
	{
		printf("服务器断开连接\n");
		exit(-1);
	}	
	if(msg.flag == 0)
	{
		printf("账号或密码错误\n");
		msg.flag = 2;	
		send(clientSocket,&msg,sizeof(MsgData),0);
		return;
	}
	else if(msg.flag == 1)
	{
		printf("账号密码正确\n");
		printf("输入想要修改成的密码:");
		gets(tempPassword);
		strcpy(msg.pass,tempPassword);
		msg.flag = 3;	
		send(clientSocket,&msg,sizeof(MsgData),0);
		if(recv(clientSocket,&msg,sizeof(MsgData),0) <= 0)//每次都判断
		{
			printf("服务器断开链接\n");
			exit(-1);
		}		
		if(msg.flag == -1)
		{
			printf("操作失败\n");
		}
		else
		{
			printf("操作完成\n");
		}		
	}
}

void retrieveAccountPassword()		//找回账号密码
{
	MsgData msg;
	msg.work = 19;	
	printf("输入昵称：");
	gets(msg.name);
	send(clientSocket,&msg,sizeof(MsgData),0);	
	if(recv(clientSocket,&msg,sizeof(MsgData),0) <= 0)//每次都判断
	{
		printf("服务器断开链接\n");
		exit(-1);
	}		
	if(msg.flag != 1)
	{
		printf("查无此人\n");
		return;		
	}
	msg.work = 20;
	printf("密保问题：%s\n",msg.question);
	printf("请输入答案：");
	gets(msg.answer);	
	send(clientSocket,&msg,sizeof(MsgData),0);
	if(recv(clientSocket,&msg,sizeof(MsgData),0) <= 0)//每次都判断
	{
		printf("服务器断开链接\n");
		exit(-1);
	}		
	if(msg.flag == 1)
	{
		printf("你的账号为%s\n",msg.account);
		printf("你的密码为%s\n",msg.pass);
	}
	else
	{
		printf("答案错误\n");
	}
}


//现在还差图形界面没有，好烦


