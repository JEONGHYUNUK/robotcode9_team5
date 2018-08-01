#include <pigpiod_if2.h>
#include <stdio.h>
#include <unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<fcntl.h>

#define INPUT1 5
#define INPUT2 6
#define INPUT3 17
#define INPUT4 27
#define DO_L 26
#define DO_R 19

int stop_pwm(int pi);
int go_pwm(int pi,int range);
void cb_func_encor1(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_encor2(int pi, unsigned gpio, unsigned level, uint32_t tick);
uint32_t start_encor1, dist_encor1,start_encor2,dist_encor2;
int num2,num1;
int wgt1=753; //wgt for Right
int wgt2=750; //wgt for Left
void setgpiomode(int pi,int range);

int main(void)
{

    int pi=1;
    int range= 8000;
    int i;
    int num=0;
    float rpm_sum[2],rpm_avg[2];
    float rpm[2],velocity[2];
    if((pi = pigpio_start(NULL, NULL)) < 0)
    {
        fprintf(stderr, "pigpio_start error\n");
        return 1;
    }
    printf("실행중\n");
    setgpiomode(pi,range);
    set_mode(pi,DO_L,PI_INPUT);
    set_mode(pi,DO_R,PI_INPUT);
    callback(pi,DO_L,EITHER_EDGE,cb_func_encor1); 
    callback(pi,DO_R,EITHER_EDGE,cb_func_encor2);
    while(1)
    {
        go_pwm(pi,range);
       
       printf("main wgt1 == %d, wgt2 == %d\n", wgt1, wgt2);
       printf("num1 : %d, num2 : %d\n",num1, num2);

       while(40 < num1)
       {
           gpio_write(pi, INPUT1, PI_LOW);
           gpio_write(pi, INPUT4, PI_LOW);
           set_PWM_dutycycle(pi, INPUT2, (range*wgt2/1000));
           gpio_write(pi, INPUT3, PI_LOW);
           if(41 < num2)
           {
               stop_pwm(pi);
               num1 = 1;
               num2 = 0;
               break;
           }
       }
       while(40 < num2)
       {
           printf("4\n");
           gpio_write(pi, INPUT1, PI_LOW);
           gpio_write(pi, INPUT4, PI_LOW);
           set_PWM_dutycycle(pi, INPUT3, (range*wgt1/1000));
           gpio_write(pi, INPUT2, PI_LOW);
           if(41 < num1)
           {
               stop_pwm(pi);
               num1 = 0;
               num2 = 1;
               break;
           }
       }
    }

}

void cb_func_encor1(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
        start_encor1 = tick;
    else if(level == PI_LOW)
        dist_encor1 = tick - start_encor1;
    num1++;
}

void cb_func_encor2(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
        start_encor2 = tick;
    else if(level == PI_LOW)
        dist_encor2 = tick - start_encor2;
    num2++;
}

int go_pwm(int pi,int range)
{
    set_PWM_dutycycle(pi,INPUT2,(range*wgt1/1000)); gpio_write(pi,INPUT1,PI_LOW);
    set_PWM_dutycycle(pi,INPUT3,(range*wgt2/1000)); gpio_write(pi,INPUT4,PI_LOW);
    printf("wgt1 == %d, wgt2 == %d\n", wgt1, wgt2);
    return 1;
}

void setgpiomode(int pi,int range)
{
    set_mode(pi, INPUT1, PI_OUTPUT); set_mode(pi, INPUT2, PI_OUTPUT);
    set_mode(pi, INPUT3, PI_OUTPUT); set_mode(pi, INPUT4, PI_OUTPUT);
    set_PWM_range(pi,INPUT1,range);    set_PWM_range(pi,INPUT2,range);
    set_PWM_range(pi,INPUT3,range);    set_PWM_range(pi,INPUT4,range);
}

int stop_pwm(int pi)
{
    gpio_write(pi, INPUT1, PI_LOW); gpio_write(pi, INPUT2, PI_LOW);
    gpio_write(pi, INPUT3, PI_LOW); gpio_write(pi, INPUT4, PI_LOW);
    return 1;
}
