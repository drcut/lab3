#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib> 
#include <string>
using namespace std;
unsigned int lat[110];
int main()
{
	memset(lat,0,sizeof(lat));
	FILE*fp = NULL;	//需要注意
  	fp = fopen("hit_latency.txt","r");
  	//int t1,t2,t3;
  	char *t1 = new char[5];
  	char *t2 = new char[5];
  	char *t3 = new char[5];
  	int index = 0;
  	while(fscanf(fp,"%s %s %s \n",t1,t2,t3)!=EOF)
  	{
  		if(t3[1]=='e')
  			t3[0]='0';
  		lat[index++]=atoi(t3);
  	}
	for(int i =0;i<index;i++)
		cout<<lat[i]<<endl;
	fclose(fp);
	return 0;
}