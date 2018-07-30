#include <stdio.h>
#include <pigpiod_if2.h>
#include<stdio.h>
#include<termios.h>
#include<stdlib.h>
#define INPUT1 5
#define INPUT2 6
#define INPUT3 17
#define INPUT4 27


void manual_ctrl(int pi,int range);
void setgpiomode(int pi,int range);
int leftcontrol(int pi,int range);
int rightcontrol(int pi,int range);
int gocontrol(int pi,int range);
int backcontrol(int pi,int range);
int stopcontrol(int pi);

int main(void)
{
    static struct termios oldtio, newtio;
    tcgetattr(0, &oldtio); // 0=standard input
    newtio = oldtio;
    newtio.c_lflag &= ~ICANON; //ICANON 정규입력 프로세싱 모드
    newtio.c_lflag &= ~ECHO; // ECHO 입력문자 반향
    tcsetattr(0, TCSANOW, &newtio);//TCSANOW 즉시 속성 변경
     
    int pi; 
    int range=3000;
    if((pi = pigpio_start(NULL, NULL)) < 0)
    {
        fprintf(stderr, "%s\n", pigpio_error(pi)); exit(-1);
    }
    printf("start mot\n");
    setgpiomode(pi,range);
    manual_ctrl(pi,range);  
 
    tcsetattr(0,TCSANOW,&oldtio);
    return 0;
}


void manual_ctrl(int pi,int range)
{
    int c;
    setgpiomode(pi,range);
    while((c = getc(stdin)) != EOF){
        switch(c){
            case 32:
                {
                    printf("<STOP>\n");
                    stopcontrol(pi);
                    break;
                }
            case 27:
                c = getc(stdin);
                if(c == 91) {
                    c = getc(stdin);
                    if(c == 67)
                    {
                        printf("<RIGHT>\n");
                        rightcontrol(pi,range);
                    }
                    else if(c == 68)
                    {
                        printf("<LEFT>\n");
                        leftcontrol(pi,range);
                    }
                    else if(c == 66)
                    {
                        printf("<BACK>\n");
                        backcontrol(pi,range);
                    }
                    else if(c == 65)
                    {
                        printf("<GO>\n");
                        gocontrol(pi,range);
                    }
                }
                break;
        }
        stopcontrol(pi);
    }
    if(ferror(stdin))
    {
        printf("Input error\n");
    }
}



void setgpiomode(int pi,int range)
{   
    set_mode(pi, INPUT1, PI_OUTPUT); set_mode(pi, INPUT2, PI_OUTPUT);
    set_mode(pi, INPUT3, PI_OUTPUT); set_mode(pi, INPUT4, PI_OUTPUT);
    set_PWM_range(pi,INPUT1,range);    set_PWM_range(pi,INPUT2,range);
    set_PWM_range(pi,INPUT3,range);    set_PWM_range(pi,INPUT4,range);
}

int leftcontrol(int pi,int range)
{
    int i;
    for(i=range*0.4;i<range*0.7;i++)
    {
        set_PWM_dutycycle(pi,INPUT2,i); gpio_write(pi,INPUT1,PI_LOW);
        set_PWM_dutycycle(pi,INPUT4,i); gpio_write(pi,INPUT3,PI_LOW);
    }
    return 1;
}

int rightcontrol(int pi,int range)
{
    int i;
    for(i=range*0.40;i<range*0.7;i++)
    {
        set_PWM_dutycycle(pi,INPUT1,i); gpio_write(pi,INPUT2,PI_LOW);
        set_PWM_dutycycle(pi,INPUT3,i); gpio_write(pi,INPUT4,PI_LOW);
    }
    return 1;
}

int gocontrol(int pi,int range)
{         
    int i;
    for(i=range*0.4;i<range*0.7;i++)
    {
        set_PWM_dutycycle(pi,INPUT2,i); gpio_write(pi,INPUT1,PI_LOW);
        set_PWM_dutycycle(pi,INPUT3,i); gpio_write(pi,INPUT4,PI_LOW);
    }
    return 1;
}

int backcontrol(int pi,int range)
{
    int i;
    for(i=range*0.4;i<range*0.7;i++)
    {
        set_PWM_dutycycle(pi,INPUT1,i); gpio_write(pi,INPUT2,PI_LOW);
        set_PWM_dutycycle(pi,INPUT4,i); gpio_write(pi,INPUT3,PI_LOW);
    }
    return 1;
}

int stopcontrol(int pi)
{
    gpio_write(pi, INPUT1, PI_LOW); gpio_write(pi, INPUT2, PI_LOW);
    gpio_write(pi, INPUT3, PI_LOW); gpio_write(pi, INPUT4, PI_LOW);
    return 1;
}


