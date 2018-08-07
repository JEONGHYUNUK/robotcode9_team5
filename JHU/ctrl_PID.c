#include <pigpiod_if2.h>
#include <stdio.h>
#include <unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<fcntl.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#define INPUT1 5    //R_R
#define INPUT2 6    //R_F
#define INPUT3 17   //L_F
#define INPUT4 27   //L_R
#define DO_L 26
#define DO_R 19

int go_pwm(int pi,int range);
void cb_func_encor1(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_encor2(int pi, unsigned gpio, unsigned level, uint32_t tick);
uint32_t start_encor1, dist_encor1,start_encor2,dist_encor2;
int num2,num1;
int wgt1=6000; //wgt for Right
int wgt2=6000; //wgt for Left
void setgpiomode(int pi,int range);
int stop_pwm(int pi);

double num[2];
double control[2];

int main(void)
{
    int pi=1;
    int range= 1000;
    int i;

    double err[2],prev_err[2],sum_err[2],ang[2],sum_ang[2]; //현재 이전오차
    double I_err[2],D_err[2]; //오차적분, 오차미분
    double Kp_term[2],Ki_term[2],Kd_term[2]; //p항,i항,d항
    sum_ang[0]=0;
    sum_err[0]=0;
    sum_err[1]=0;
    double Kp1 =0.1;
    double Ki1=0;
    double Kd1=0;
    double S=0;
    control[0]=0;
    Kp_term[0] = 0;
    Ki_term[0] = 0;        
    Kd_term[0] = 0;   
    int current_time, last_time;
    double dt = 1;
    if((pi = pigpio_start(NULL, NULL)) < 0)
    {
        fprintf(stderr, "pigpio_start error\n");
        return 1;
    }
    printf("실행중\n");
    setgpiomode(pi,range);
    set_mode(pi,DO_L,PI_INPUT);
    set_mode(pi,DO_R,PI_INPUT);
    callback(pi,DO_R,EITHER_EDGE,cb_func_encor1);
    callback(pi,DO_L,EITHER_EDGE,cb_func_encor2);

    while(1)
    {
        go_pwm(pi,range);
          if(num[0]>num[1])
            S = 3.141592*6*num[1]/40;
        else if(num[1]>num[0])
            S = 3.141592*6*num[0]/40;
        current_time=time_time(); 
        
       // printf("확인1번 겟커런트까지\n");

        if((last_time+1)<=current_time)
        {
            last_time=current_time;
            printf("오른쪽 엔코더 읽은 횟수 : %lf\n",num[0]);
            printf("왼쪽 엔코더 읽은 횟수 : %lf\n",num[1]);
            if(num[0]>num[1])
            ang[0] =0.0942*(num[0]-num[1]); //라디안으로 표시 360도 = 2PI 6/10*2*3.14/40*


            printf("ang: %lf \n" , ang[0]);
            sum_ang[0] += ang[0];
            err[0] = S*sin(6.28+0.6*sum_ang[0]);
            printf("err: %lf \n" , err[0]);
            sum_err[0] += err[0];
            printf("sum_err: %lf \n" , sum_err[0]);
            Kp_term[0] = Kp1*sum_err[0];
            Ki_term[0] = Ki1*sum_err[0]*dt;
            Kd_term[0] = Kd1*(sum_err[0]-prev_err[0])/dt;
            control[0] = Kp_term[0] + Ki_term[0] + Kd_term[0];
            printf("sum_err[0] : %lf   sum_ang[0]: %lf \n",sum_err[0],sum_ang[0]);
            printf("control1 : %lf Kp1 : %lf Kd1 : %lf Ki1 : %lf \n",control[0],Kp_term[0],Kd_term[0],Ki_term[0]);
            printf("wgt1 : %d wgt2: %d \n",wgt1,wgt2);
            wgt1 -=control[0];  //right
            wgt2 +=control[0];   //left
            num[0]=0;
            num[1]=0;
            prev_err[0] = sum_err[0];
        }

        /*
           ang[1] = 6/10*2*PI/40*(num[0]-num[1]); //라디안으로 표시 360도 = 2PI
           sum_ang[1] += ang[1];
           err[1] = S*sin(6/10*sum_ang[1]);
           Kp_term[1] = Kp2*err[1];
           Ki_term[1] = Ki2*err[1]*dt;
           Kd_term[1] = (err[1]-prev_err[1])*Kd2;
           sum_err[1] += err[1];
           control[1] = Kp_term[1] + Ki_term[1] + Kd_term[1];
         */
        //       printf("control2 : %lf Kp2 : %lf Kd2 : %lf Ki2 : %lf \n",control2,Kp_term2,Kd_term2,Ki_term2);

        else if(num[0]>3000||num[1]>3000)
            printf("INPUT ERROR!!!!\n");
        else if(  dist_encor2>=100000000 &&dist_encor1>=100000000)
        { 
            printf("정지\n");
        }
    }

}

void cb_func_encor1(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
        start_encor1 = tick;
    else if(level == PI_LOW)
        dist_encor1 = tick - start_encor1;
    num[0]++;
}

void cb_func_encor2(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
        start_encor2 = tick;
    else if(level == PI_LOW)
        dist_encor2 = tick - start_encor2;
    num[1]++;
}

int go_pwm(int pi,int range)
{
    set_PWM_dutycycle(pi,INPUT2,(range*wgt1/10000)); gpio_write(pi,INPUT1,PI_LOW);
    set_PWM_dutycycle(pi,INPUT3,(range*wgt2/10000)); gpio_write(pi,INPUT4,PI_LOW);
    return 1;
}

void setgpiomode(int pi,int range)
{
    set_mode(pi, INPUT1, PI_OUTPUT); set_mode(pi, INPUT2, PI_OUTPUT);
    set_mode(pi, INPUT3, PI_OUTPUT); set_mode(pi, INPUT4, PI_OUTPUT);
    set_PWM_range(pi,INPUT1,range);    set_PWM_range(pi,INPUT2,range);
    set_PWM_range(pi,INPUT3,range);    set_PWM_range(pi,INPUT4,range);
    set_glitch_filter(pi,INPUT1,8300);    set_glitch_filter(pi,INPUT2,8300);
    set_glitch_filter(pi,INPUT3,8300);   set_glitch_filter(pi,INPUT4,8300);
}

int stop_pwm(int pi)
{
    gpio_write(pi, INPUT1, PI_LOW); gpio_write(pi, INPUT2, PI_LOW);
    gpio_write(pi, INPUT3, PI_LOW); gpio_write(pi, INPUT4, PI_LOW);
    return 1;
}

