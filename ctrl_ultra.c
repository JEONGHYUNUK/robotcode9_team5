#include <stdio.h>
#include <pigpiod_if2.h>
#include<stdio.h>
#include<termios.h>
#include<stdlib.h>
#define INPUT1 5
#define INPUT2 6
#define INPUT3 17
#define INPUT4 27
#define TRIG_PINNO 20 
#define ECHO_PINNO 16


void trigger(void);
void manual_ctrl(int pi,int range);
void setgpiomode(int pi,int range);
int left_pwm(int pi,int range);
int right_pwm(int pi,int range);
int go_pwm(int pi,int range);
int back_pwm(int pi,int range);
int stop_pwm(int pi);
int go_ctrl(int pi);


void cb_func_echo(int pi, unsigned gpio, unsigned level, uint32_t tick);
uint32_t start_tick_, dist_tick_;

int main(void)
{
    static struct termios oldtio, newtio;
    tcgetattr(0, &oldtio); // 0=standard input
    newtio = oldtio;
    newtio.c_lflag &= ~ICANON; //ICANON 정규입력 프로세싱 모드
    newtio.c_lflag &= ~ECHO; // ECHO 입력문자 반향
    tcsetattr(0, TCSANOW, &newtio);//TCSANOW 즉시 속성 변경
     
    int pi,cha; 
    int num1=0;
    int num2=0;
    int range=4000;
    float distance;
    if((pi = pigpio_start(NULL, NULL)) < 0)
    {
        fprintf(stderr, "%s\n", pigpio_error(pi)); exit(-1);
    }
    printf("start mot\n");
    callback(pi, ECHO_PINNO, EITHER_EDGE, cb_func_echo);
    printf("start manual mode press number 1\nstart auto mode press number 2\n");
    scanf("%d",&num2);
    if(num2 == 2)
    {
        while(1)
        {       
            setgpiomode(pi,range);
            start_tick_ = dist_tick_ = 0;
            gpio_trigger(pi, TRIG_PINNO, 10, PI_HIGH);
            time_sleep(0.05);
            if(dist_tick_ && start_tick_)
            {
                distance = dist_tick_ / 1000000. * 340 / 2 * 100;
                if(distance < 8)
                {
                    printf("8cm  이내\n");
                    stop_pwm(pi);
                    num1=0;
                }
                else if(distance >=8)
                {
                    printf("interval : %6dus, Distance : %6.1f cm\n", dist_tick_, distance);
                    while( num1==0)
                    {
                        num1++;
                        go_pwm(pi,range);
                    }
                    if(num1>=1)
                        go_ctrl(pi);
                } 
            }
            else
                printf("sense error\n");
            time_sleep(0.05);
        }
    }
    else if(num2 ==1)
        manual_ctrl(pi,range);

    pigpio_stop(pi);
    tcsetattr(0,TCSANOW,&oldtio);
    return 0;
}


void manual_ctrl(int pi,int range)
{
    int c;
    setgpiomode(pi,range);
    while((c = getc(stdin)) != EOF){
        switch(c)
        {
            case 32:
                {
                    printf("<STOP>\n");
                    stop_pwm(pi);
                    break;
                }
            case 27:
                c = getc(stdin);
                if(c == 91) {
                    c = getc(stdin);
                    if(c == 67)
                    {
                        printf("<RIGHT>\n");
                        right_pwm(pi,range);
                    }
                    else if(c == 68)
                    {
                        printf("<LEFT>\n");
                        left_pwm(pi,range);
                    }
                    else if(c == 66)
                    {
                        printf("<BACK>\n");
                        back_pwm(pi,range);
                    }
                    else if(c == 65)
                    {
                        printf("<GO>\n");
                        go_pwm(pi,range);
                    }
                    else
                        stop_pwm(pi);
                }
                break;
        }
    }
    if(ferror(stdin))
    {
        printf("Input error\n");
    }
}

void cb_func_echo(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
        start_tick_ = tick;
    else if(level == PI_LOW)
        dist_tick_ = tick - start_tick_;
}

void setgpiomode(int pi,int range)
{   
    set_mode(pi, INPUT1, PI_OUTPUT); set_mode(pi, INPUT2, PI_OUTPUT);
    set_mode(pi, INPUT3, PI_OUTPUT); set_mode(pi, INPUT4, PI_OUTPUT);
    set_PWM_range(pi,INPUT1,range);    set_PWM_range(pi,INPUT2,range);
    set_PWM_range(pi,INPUT3,range);    set_PWM_range(pi,INPUT4,range);
    set_mode(pi, TRIG_PINNO, PI_OUTPUT);
    set_mode(pi, ECHO_PINNO, PI_INPUT);
    gpio_write(pi, TRIG_PINNO, PI_OFF);
}

int left_pwm(int pi,int range)
{
    int i;
    for(i=range*0.4;i<range*0.7;i++)
    {
        set_PWM_dutycycle(pi,INPUT2,i); gpio_write(pi,INPUT1,PI_LOW);
        set_PWM_dutycycle(pi,INPUT4,i); gpio_write(pi,INPUT3,PI_LOW);
    }
    return 1;
}

int right_pwm(int pi,int range)
{
    int i;
    for(i=range*0.40;i<range*0.7;i++)
    {
        set_PWM_dutycycle(pi,INPUT1,i); gpio_write(pi,INPUT2,PI_LOW);
        set_PWM_dutycycle(pi,INPUT3,i); gpio_write(pi,INPUT4,PI_LOW);
    }
    return 1;
}

int go_pwm(int pi,int range)
{         
    int i;
    for(i=range*0.4;i<range*0.7;i++)
    {
        set_PWM_dutycycle(pi,INPUT2,i); gpio_write(pi,INPUT1,PI_LOW);
        set_PWM_dutycycle(pi,INPUT3,i); gpio_write(pi,INPUT4,PI_LOW);
    }
    return 1;
}

int back_pwm(int pi,int range)
{
    int i;
    for(i=range*0.4;i<range*0.7;i++)
    {
        set_PWM_dutycycle(pi,INPUT1,i); gpio_write(pi,INPUT2,PI_LOW);
        set_PWM_dutycycle(pi,INPUT4,i); gpio_write(pi,INPUT3,PI_LOW);
    }
    return 1;
}

int stop_pwm(int pi)
{
    gpio_write(pi, INPUT1, PI_LOW); gpio_write(pi, INPUT2, PI_LOW);
    gpio_write(pi, INPUT3, PI_LOW); gpio_write(pi, INPUT4, PI_LOW);
    return 1;
}

int go_ctrl(int pi)
{
    gpio_write(pi,INPUT1,PI_LOW);   gpio_write(pi,INPUT2,PI_HIGH);
    gpio_write(pi,INPUT3,PI_HIGH);  gpio_write(pi,INPUT4,PI_LOW);
}
