#include <stdio.h>
#include <pigpiod_if2.h>
#include<stdio.h>
#include<termios.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#define INPUT1 5
#define INPUT2 6
#define INPUT3 17
#define INPUT4 27
#define TRIG_PINNO1 20 
#define ECHO_PINNO1 16
#define TRIG_PINNO2 23
#define ECHO_PINNO2 24
#define TRIG_PINNO3 2
#define ECHO_PINNO3 3

void trigger(void);
void auto_ctrl(int pi, int range);
void manual_ctrl(int pi,int range);
void setgpiomode(int pi,int range);
int left_pwm(int pi,int range);
int right_pwm(int pi,int range);
int go_pwm(int pi,int range);
int back_pwm(int pi,int range);
int stop_pwm(int pi);
int go_ctrl(int pi,int range);
void init_keyboard();
void close_keyboard();
int _kbhit();
int _getch();
int _putch();

void cb_func_encor1(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_encor2(int pi, unsigned gpio, unsigned level, uint32_t tick);
uint32_t start_encor1, dist_encor1,start_encor2,dist_encor2;
void cb_func_echo(int pi, unsigned gpio, unsigned level, uint32_t tick);
uint32_t start_tick_, dist_tick_;


static struct termios initial_settings, new_settings;
static int peek_character = -1;

int wgt1=753; //wgt for Right
int wgt2=750; //wgt for Left
int range=4000;

int main(void)
{
    static struct termios oldtio, newtio;
    tcgetattr(0, &oldtio); // 0=standard input
    newtio = oldtio;
    newtio.c_lflag &= ~ICANON; //ICANON 정규입력 프로세싱 모드
    newtio.c_lflag &= ~ECHO; // ECHO 입력문자 반향
    tcsetattr(0, TCSANOW, &newtio);//TCSANOW 즉시 속성 변경
     

    int pi, num2=0;
    if((pi = pigpio_start(NULL, NULL)) < 0)
    {
        fprintf(stderr, "%s\n", pigpio_error(pi)); exit(-1);
    }
    printf("start mot\n");
    callback(pi, ECHO_PINNO1, EITHER_EDGE, cb_func_echo);
    callback(pi, ECHO_PINNO2, EITHER_EDGE, cb_func_echo);
    callback(pi, ECHO_PINNO3, EITHER_EDGE, cb_func_echo);
    printf("start manual mode press number 1\nstart auto mode press number 2\n");
    scanf("%d",&num2);
    

    while(1)
    {
        if(num2 == 2)
        {
            auto_ctrl(pi, range);
            num2 = 1;
        }
         if(num2 == 1){
            manual_ctrl(pi,range);
            printf("mode change(m >> a)\n");
            stop_pwm(pi);
            num2 = 2;
            tcsetattr(0,TCSANOW,&oldtio);
        }
    }
}

void auto_ctrl(int pi, int range)
{
    float distance1, distance2= 10, distance3=10;
  init_keyboard();
            while(1)
            {  
                {
                setgpiomode(pi,range);
                gpio_trigger(pi, TRIG_PINNO1, 5, PI_HIGH);
                time_sleep(0.005);
                if(dist_tick_ && start_tick_)
                {
                    distance1 = dist_tick_ / 1000000. * 340 / 2 * 100;
                    if(distance1 < 8)
                    {
                        printf("정면 8cm 이내\n");
                        stop_pwm(pi);
                        if((distance2 < 8)&&(distance3 < 8))
                        {
                            back_pwm(pi,range);
                            time_sleep(0.005); 
                        }
                        else if(distance2 < 8)
                        {
                            right_pwm(pi,range);
                            time_sleep(0.005);
                            stop_pwm(pi);
                        }
                        else if(distance3 < 8)
                        {
                            left_pwm(pi,range);
                            time_sleep(0.005);
                            stop_pwm(pi);
                        }
                    }
                    else if(distance1 >=8)
                    {
                        printf("정면 초음파 센서 \n interval : %6dus, Distance : %6.1f cm\n", dist_tick_, distance1);
                        go_pwm(pi,range);
                    }
                }
                else
                    printf("sense error\n");
                time_sleep(0.05);
                }
                {
                   
                setgpiomode(pi,range);
                gpio_trigger(pi, TRIG_PINNO2, 10, PI_HIGH);
                time_sleep(0.05);
                if(dist_tick_ && start_tick_)
                {
                    distance2 = dist_tick_ / 1000000. * 340 / 2 * 100;
                    if(distance2 < 8)
                    {
                        printf("왼쪽 8cm 이내\n");
                    }
                    else if(distance2 >=8)
                    {
                        printf("왼쪽 초음파 센서 \n interval : %6dus, Distance : %6.1f cm\n", dist_tick_, distance2);
                    }
                }
                else
                    printf("sense error\n");
                time_sleep(0.05);

                }

                {  
                setgpiomode(pi,range);
                gpio_trigger(pi, TRIG_PINNO3, 10, PI_HIGH);
                time_sleep(0.05);
                if(dist_tick_ && start_tick_)
                {
                    distance3 = dist_tick_ / 1000000. * 340 / 2 * 100;
                    if(distance3 < 8)
                    {
                        printf("오른쪽 8cm 이내\n");
                    }
                    else if(distance3 >=8)
                    {
                        printf("오른쪽 초음파 센서 \n interval : %6dus, Distance : %6.1f cm\n", dist_tick_, distance3);
                    }
                }
                else
                    printf("sense error\n");
                time_sleep(0.05);



                }
            }
}

void manual_ctrl(int pi,int range)
{
    int c;
    setgpiomode(pi,range);
    while((c = getc(stdin)) != 97){
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
    set_mode(pi, TRIG_PINNO1, PI_OUTPUT);
    set_mode(pi, ECHO_PINNO1, PI_INPUT);
    set_mode(pi, TRIG_PINNO2, PI_OUTPUT);
    set_mode(pi, ECHO_PINNO2, PI_INPUT); 
    set_mode(pi, TRIG_PINNO3, PI_OUTPUT);
    set_mode(pi, ECHO_PINNO3, PI_INPUT);
    gpio_write(pi, TRIG_PINNO1, PI_OFF);
    gpio_write(pi, TRIG_PINNO2, PI_OFF);
    gpio_write(pi, TRIG_PINNO3, PI_OFF);
}

int left_pwm(int pi,int range)
{ 
    set_PWM_dutycycle(pi,INPUT2,(range*wgt1/1000)); gpio_write(pi,INPUT1,PI_LOW);
    set_PWM_dutycycle(pi,INPUT4,(range*wgt2/1000)); gpio_write(pi,INPUT3,PI_LOW);
    return 1;
}

int right_pwm(int pi,int range)
{
    set_PWM_dutycycle(pi,INPUT1,(range*wgt1/1000)); gpio_write(pi,INPUT2,PI_LOW);
    set_PWM_dutycycle(pi,INPUT3,(range*wgt2/1000)); gpio_write(pi,INPUT4,PI_LOW);
    return 1;
}


int back_pwm(int pi,int range)
{
    set_PWM_dutycycle(pi,INPUT1,(range*wgt1/1000)); gpio_write(pi,INPUT2,PI_LOW);
    set_PWM_dutycycle(pi,INPUT4,(range*wgt2/1000)); gpio_write(pi,INPUT3,PI_LOW);
    return 1;
}

int stop_pwm(int pi)
{
    gpio_write(pi, INPUT1, PI_LOW); gpio_write(pi, INPUT2, PI_LOW);
    gpio_write(pi, INPUT3, PI_LOW); gpio_write(pi, INPUT4, PI_LOW);
    return 1;
}

int go_pwm(int pi,int range)
{
    set_PWM_dutycycle(pi,INPUT2,(range*wgt1/1000)); gpio_write(pi,INPUT1,PI_LOW);
    set_PWM_dutycycle(pi,INPUT3,(range*wgt2/1000)); gpio_write(pi,INPUT4,PI_LOW);
    return 1;
}

void init_keyboard()
{
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}

void close_keyboard()
{
    tcsetattr(0, TCSANOW, &initial_settings);
}

int _kbhit()
{
    unsigned char ch;
    int nread;
    if (peek_character != -1) return 1;
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    new_settings.c_cc[VMIN]=1;
    tcsetattr(0, TCSANOW, &new_settings);
    if(read(0,&ch,1) == 1) 
    {
        peek_character = ch;
        return 1;
    }
    return 0;
}

int _getch()
{
    char ch;
    if(peek_character != -1) 
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}

int _putch(int c) {
    putchar(c);
    fflush(stdout);
    return c;
}


void cb_func_encor1(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
        start_encor1 = tick;
    else if(level == PI_LOW)
        dist_encor1 = tick - start_encor1;
}

void cb_func_encor2(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
        start_encor2 = tick;
    else if(level == PI_LOW)
        dist_encor2 = tick - start_encor2;
}

