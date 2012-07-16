#include<iostream>
#include<stdio.h>
using namespace std;
#include<cstdlib>
#include<queue>
#include<pthread.h>
#include<ctime>

#define NUM_CELLS 3
#define NUM_USERS 10
#define NUM_PRS 2
#define NUM_STATIC 2
#define NUM_DYNAMIC 5
#define NUM_USERS_PER_PRS 5
#define MAX_TIME 10
#define MAX_WAIT 2

int number_of_discarded,number_of_calls;
queue <int> q[NUM_CELLS];
int stat[NUM_CELLS][NUM_STATIC][2],dynamic[NUM_DYNAMIC][2];
pthread_mutex_t dy,sta[NUM_CELLS],m[NUM_CELLS];
pthread_mutex_t qu[NUM_CELLS];
int r[NUM_CELLS];
int a[NUM_CELLS][NUM_USERS][2];
queue<int> remove(queue <int>q1,int id)
{
while(q1.front()!=id)
q1.pop();
q1.pop();
return q1;
}
int search2(queue <int>q,int id)
{
queue <int>q1;
q1=q;
while(!q1.empty())
{
    if(q1.front()==id)
        return 1;
    q1.pop();
}
return 0;
}
void timer(int id,int i)
{
    int t=time(0);
    while((time(0)-t)<=MAX_WAIT)
    {
        if(search2(q[i],id)==0)
        break;
        sleep(1);
    }
    if(time(0)-t>MAX_WAIT)
    {
        number_of_discarded++;
        printf("Request from %d is discarded\n",id);
        q[i]=remove(q[i],id);
    }
}
void endcall(int id,int i)
{
    int j=0;
    for(j=0;j<NUM_STATIC;j++)
        if(stat[i][j][1]==id)
            break;
        if(j<NUM_STATIC)
        {
            stat[i][j][0]=0;
            stat[i][j][1]=0;
            printf("Releasing Static channel no %d from cell %d \n",j,i);
            int k;
            for(k=0;k<NUM_USERS;k++)
            if(a[i][k][1]==id)
            {
            a[i][k][0]=0;
            a[i][k][1]=0;
            }
            if(!q[i].empty())
            {
                printf("Allocating Static channel %d  of cell  %d to %d \n",j,i,q[i].front());
                q[i].pop();
            }
            else
            {
            printf("Allocating Static channel %d  of cell  %d to %d \n",j,i,rand());
            a[i][k][0]=1;
            a[i][k][1]=rand();
            number_of_calls++;
            }
            }
        else
        {
            for(j=0;j<NUM_DYNAMIC;j++)
                if(dynamic[j][1]==id)
                    break;
            if(j<NUM_DYNAMIC)
            {
                int rc1=pthread_mutex_trylock(&dy);
                while(rc1!=0)
                    rc1=pthread_mutex_trylock(&dy);
                dynamic[j][0]=0;
                dynamic[j][1]=0;
                pthread_mutex_unlock(&dy);
                int k1;
                for(k1=0;k1<NUM_USERS;k1++)
            if(a[i][k1][1]==id)
            {
            a[i][k1][0]=0;
            a[i][k1][1]=0;
            }
                int  l=0;
                printf("Releasing Dynamic Channel %d \n",j);
                for(l=0;l<NUM_CELLS;l++)
                {
                    if(!q[l].empty())
                        break;
                }
                if(l<NUM_CELLS)
                {

                    printf("Allocating Dynamic Channel %d to cell %d to user %d \n",j,i,q[l].front());
                    q[l].pop();
                }
                else
                {
                printf("Dynamic Channel %d is allocated to %d in cell %d \n",j,i,rand());
                a[i][k1][0]=-1;
                a[i][k1][1]=rand();
                number_of_calls++;
                }
            }
        }
}
int search(int a[NUM_CELLS][NUM_USERS][2],int i)
{

int j=0;
for(j=0;j<NUM_USERS;j++)
    if(a[i][j][0]==0)
    {
        return (j+1);
    }
    return 0;
}
int search1(int a[NUM_CELLS][NUM_USERS][2],int i)
{
for(int j=0;j<NUM_USERS;j++)
if(a[i][j][0]==-1)
return j+1;
return 0;
}
void *userfn(void  *ptr)
{
int tim;
int i=0,id=0;
int t=time(0),t1;
srand(time(0));
while(1)
{
    id=rand();
    i=rand();
    i=i%NUM_CELLS;
    t1=time(0);
    while((r[i]>=NUM_USERS-1)&&search(a,i)==0)
    ;
    {
        int rc=pthread_mutex_trylock(&m[i]);
        while(rc==16)
            rc=pthread_mutex_trylock(&m[i]);
        int t=search(a,i);
        if(t!=0)
        {

    if(r[i]<NUM_USERS-1)
        r[i]++;
        a[i][t-1][0]=1;
        a[i][t-1][1]=id;
        number_of_calls++;
        }
        int t1=search1(a,i);
        if(t1!=0)
        {
        tim=rand()%MAX_TIME;
        sleep(tim);
        endcall(id,i);
        }
        pthread_mutex_unlock(&m[i]);
    }
}

}
void *allocate(void *ptr)
{
int n=(int)ptr;
int i,j,rc1,rc2;
j=n%(NUM_PRS);//Processor no
i=n/(NUM_PRS);//Cell no
int k=0,f=(j*NUM_USERS_PER_PRS)+1,l=((j+1)*NUM_USERS_PER_PRS);//range of users to be handeled by a particular processor
if(r[i]<f)
    return((void *)0);
int t=time(0);
while(f<=l&&r[i]>=f)
{
        int rc3=pthread_mutex_trylock(&m[i]);
    while(rc3==16)
        rc3=pthread_mutex_trylock(&m[i]);
    if(stat[i][j][0]==0)
    {
        if(a[i][f-1]!=0&&a[i][f-1][0]!=-1)
        {
        stat[i][j][0]=1;
        stat[i][j][1]=a[i][f-1][1];
        printf("Static Channel %d of cell %d is allocated to %d\n",k,i,a[i][f-1][1]);
        }
        r[i]--;
        a[i][f-1][0]=-1;
        f++;
        pthread_mutex_unlock(&m[i]);
    }
    else
    {
        pthread_mutex_unlock(&m[i]);
        rc1=pthread_mutex_trylock(&dy);
        while(rc1==16)
            rc1=pthread_mutex_trylock(&dy);
        for(k=0;k<NUM_DYNAMIC;k++)
            if(dynamic[k][0]==0)
                {
                    break;
                }

            int rc4=pthread_mutex_trylock(&m[i]);
            while(rc4==16)
                rc4=pthread_mutex_trylock(&m[i]);
            if(k<NUM_DYNAMIC)
            {
                if(a[i][f-1][1]!=0&&a[i][f-1][0]!=-1)
                {
                dynamic[k][0]=1;
                dynamic[k][1]=a[i][f-1][1];
                printf("Dynamic channel %d allocated to user %d in cell %d \n",k,a[i][f-1][1],i);
                }
            f++;a[i][f-2][0]=-1;
            r[i]--;
            pthread_mutex_unlock(&dy);
            pthread_mutex_unlock(&m[i]);
            }
            else
            {
            pthread_mutex_unlock(&m[i]);
            pthread_mutex_unlock(&dy);
            rc2=16;
            while(rc2!=0)
            rc2=pthread_mutex_trylock(&qu[i]);

            int rc5=16;
    while(f<=l&&r[i]>=f)
    {
    while(rc5==16)
        rc5=pthread_mutex_trylock(&m[i]);
            if(a[i][f-1][1]!=0&&a[i][f-1][0]!=-1)
            {
            printf("User %d must wait as cell %d is busy \n",a[i][f-1][1],i);
            q[i].push(a[i][f-1][1]);
            a[i][f-1][0]=-1;
            }
            pthread_mutex_unlock(&qu[i]);
            timer(q[i].front(),i);
            f++;
            r[i]--;
    }
            f=l;
            pthread_mutex_unlock(&qu[i]);
            pthread_mutex_unlock(&m[i]);
            return((void *)0);
            }
        }
        }
        return((void *)0);
}
void *cellfn(void *ptr)
{
    pthread_t pro[NUM_PRS];
    int j=0;
    int i=(int)ptr;
    while(1)
    {
        while(r[i]==0)
        ;

        for(j=0;j<NUM_PRS;j++)
            pthread_create(&pro[j],NULL,allocate,(void *)(i*NUM_PRS+j));

        pthread_join(pro[j-1],NULL);
    }
}
int main()
{
    pthread_t cell[NUM_CELLS];
    int i=0;
    for(i=0;i<NUM_CELLS;i++)
        pthread_create(&cell[i],NULL,cellfn,(void *)i);
    for(i=0;i<NUM_CELLS;i++)
    {
        pthread_mutex_init(&m[i], NULL);
        pthread_mutex_init(&qu[i], NULL);
        pthread_mutex_init(&sta[i], NULL);
    }
    pthread_mutex_init(&dy,NULL);
    pthread_t users[NUM_USERS];
    for(i=0;i<NUM_USERS;i++)
        pthread_create(&users[i], NULL, userfn, (void *)i);
    sleep(100);
    printf("Number of calls blocked %d Number Of calls %d Call blocking probability %f\n",number_of_discarded,number_of_calls,((float)number_of_discarded/(float)number_of_calls));
    //pthread_join(users[i-1],NULL);
    //pthread_join(cell[NUM_CELLS-1],NULL);
    return 0;
}
