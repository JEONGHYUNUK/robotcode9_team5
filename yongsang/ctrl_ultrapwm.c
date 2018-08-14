#include <stdio.h>
#include <pigpiod_if2.h>
#include<termios.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<pthread.h>
#include<time.h>
//#include<bcm2835.h>
#define DO_L 26
#define DO_R 19
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
//#define SENSOR RPI_GPIO_P1_07
//#define LED RPI_GPIO_P1_15
//#define ACTIVE_VALUE 1
#define go_pin 10
#define back_pin 9
#define left_pin 11
#define right_pin 25
#define auto_pin 8
#define camera_right_left_servo 18
#define camera_up_down_servo 21
#define updown 4
#define leftright 7

void *thr_fn1(void *arg);
void trigger(void);
void auto_ctrl(int pi, int range);
void manual_ctrl(int pi,int range);
void setgpiomode(int pi,int range);
uint32_t start_encor1, dist_encor1, start_encor2, dist_encor2;
int left_pwm(int pi,int range);
int right_pwm(int pi,int range);
int go_pwm(int pi,int range);
int back_pwm(int pi,int range);
int stop_pwm(int pi);
int go_ctrl(int pi,int range);
//void human(void);
int left_encor(int pi, int range);
int right_encor(int pi, int range);
int turn_encor(int pi, int range);

void cb_func_encor1(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_encor2(int pi, unsigned gpio, unsigned level, uint32_t tick);
uint32_t start_encor1, dist_encor1,start_encor2,dist_encor2;
void cb_func_echo1(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_echo2(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_echo3(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_go(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_back(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_left(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_right(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_auto(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_updown(int pi, unsigned gpio, unsigned level, uint32_t tick);
void cb_func_leftright(int pi, unsigned gpio, unsigned level, uint32_t tick);
void camera_turn_left(int pi);
void camera_up(int pi);
void camera_down(int pi);
void camera_turn_right(int pi);
void camera_updown(int pi);
void camera_leftright(int pi);
uint32_t start_tick_1, dist_tick_1;
uint32_t start_tick_2, dist_tick_2;
uint32_t start_tick_3, dist_tick_3;

int key, camera;
int wgt1=780; //wgt for Right
int wgt2=650; //wgt for Left
int range=8000;
int num3 = 0, num4 = 0;
int pulse1=1500, pulse2=1500;

int main(void)
{
  //  human();
    void *tret;
    pthread_t tid1;
    int err;
    int i;
    static struct termios oldtio, newtio;
    tcgetattr(0, &oldtio); // 0=standard input
    newtio = oldtio;
    newtio.c_lflag &= ~ICANON; //ICANON 정규입력 프로세싱 모드
    newtio.c_lflag &= ~ECHO; // ECHO 입력문자 반향
    tcsetattr(0, TCSANOW, &newtio);//TCSANOW 즉시 속성 변경
     
    int pi;
    if((pi = pigpio_start(NULL, NULL)) < 0)
    {
        fprintf(stderr, "%s\n", pigpio_error(pi)); exit(-1);
    }
    printf("start mot\n");
    callback(pi, ECHO_PINNO1, EITHER_EDGE, cb_func_echo1);
    callback(pi, ECHO_PINNO2, EITHER_EDGE, cb_func_echo2);
    callback(pi, ECHO_PINNO3, EITHER_EDGE, cb_func_echo3);
    callback(pi, go_pin, EITHER_EDGE, cb_func_go);
    callback(pi, back_pin, EITHER_EDGE, cb_func_back);
    callback(pi, left_pin, EITHER_EDGE, cb_func_left);
    callback(pi, right_pin, EITHER_EDGE, cb_func_right);
    callback(pi, auto_pin, EITHER_EDGE, cb_func_auto);
    callback(pi, updown, EITHER_EDGE, cb_func_updown);
    callback(pi, leftright, EITHER_EDGE, cb_func_leftright);
    printf("start manual mode\n");
    set_servo_pulsewidth(pi, camera_right_left_servo, pulse1);
    set_servo_pulsewidth(pi, camera_up_down_servo, pulse2);
    err = pthread_create(&tid1, NULL, thr_fn1, NULL);
     printf("input 1\n");
     scanf("%d",&i);
     if(i == 1)
         manual_ctrl(pi,range);

}

void auto_ctrl(int pi, int range)
{
    void *tret;
    float distance1, distance2, distance3;
    setgpiomode(pi,range);
    int SNR_D = 10;
    while(key == 6)
    {
        camera_updown(pi);
        camera_leftright(pi);
 

            if(camera == 1)
            {
                camera_turn_left(pi);
                camera = 0;
            }
            if(camera == 2)
            {
                camera_turn_right(pi);
                camera = 0;
            }
            if(camera == 3)
            {
                camera_up(pi);
                camera = 0;
            }
            if(camera == 4)
            {
                camera_down(pi);
                camera = 0;
            }        gpio_trigger(pi, TRIG_PINNO1, 10, PI_HIGH);
        gpio_trigger(pi, TRIG_PINNO2, 10, PI_HIGH);
        gpio_trigger(pi, TRIG_PINNO3, 10, PI_HIGH);
        time_sleep(0.05);
        if((dist_tick_1 && start_tick_1) && (dist_tick_2 && start_tick_2) && (dist_tick_3 && start_tick_3))
        {
            distance1 = dist_tick_1 / 1000000. * 340 / 2 * 100;
            distance2 = dist_tick_2 / 1000000. * 340 / 2 * 100;
            distance3 = dist_tick_3 / 1000000. * 340 / 2 * 100;
            if(distance1 < SNR_D)
            {
                printf("정면 50cm 이내\n");
                stop_pwm(pi);
                if((distance2>=SNR_D)&&(distance3>=SNR_D))
                {
                    printf("기본값 오른쪽으로 회전\n");
                    right_encor(pi,range);
                    key = 0;
                }

            }
            if((distance1<SNR_D)&&(distance2 < SNR_D)&&(distance3 < SNR_D))
            {
                printf("정면,좌,우모두50cm 이내\n");
                turn_encor(pi,range);
                key = 0;
            }
            else if((distance1<SNR_D)&& (distance2 < SNR_D))
            {
                printf("정면,왼쪽  50cm 이내\n");
                right_encor(pi,range);
                key = 0;
            }
            else if((distance1<SNR_D)&& (distance3 < SNR_D))
            {
                printf("정면,오른쪽  50cm 이내\n");
                left_encor(pi,range);
                key = 0;
            }
            else if(distance1 >=SNR_D)
            {
                printf("Distance : 정면 :  %6.1f 왼쪽 :  %6.1f 오른쪽:%6.1f cm\n", distance1,distance2,distance3);
                go_pwm(pi,range);
            }
        else
            printf("sense error\n");
        }
    }
}

void manual_ctrl(int pi,int range)
{
    int c;
    void *tret;
    setgpiomode(pi,range);

    while(1)
    {
        while((c = getc(stdin)) != EOF){
            switch(c)
            {
                case 97:
                    {
                        printf("auto mode running\n");
                        num3 = 0;
                        num4 = 0;
                        key = 6;
                        break;
                    }
                case 115:
                    {
                        printf("camera turn left\n");
                        camera = 1;
                        break;
                    }
                case 102:
                    {
                        printf("camera turn right\n");
                        camera = 2;
                        break;
                    }

                case 101:
                    {
                        printf("camera up\n");
                        camera = 3;
                        break;
                    }

                case 100:
                    {
                        printf("camera down\n");
                        camera = 4;
                        break;
                    }
                case 32:
                    {
                        printf("<STOP>\n");
                        num3 = 0;
                        num4 = 0;
                        key = 5;
                        break;
                    }
                case 27:
                    c = getc(stdin);
                    if(c == 91) {
                        c = getc(stdin);
                        if(c == 67)
                        {
                            printf("<RIGHT>\n");
                            num3 = 0;
                            num4 = 0;
                            key = 4;
                            break;
                        }
                        else if(c == 68)
                        {
                            printf("<LEFT>\n");
                            num3 = 0;
                            num4 = 0;
                            key = 3;
                            break;
                        }
                        else if(c == 66)
                        {
                            printf("<BACK>\n");
                            num3 = 0;
                            num4 = 0;
                            key = 2;
                            break;
                        }
                        else if(c == 65)
                        {
                            printf("<GO>\n");
                            num3 = 0;
                            num4 = 0;
                            key = 1;
                            break;
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
}

void cb_func_echo1(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
        start_tick_1 = tick;
    else if(level == PI_LOW)
        dist_tick_1 = tick - start_tick_1;
}

void cb_func_echo2(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
        start_tick_2 = tick;
    else if(level == PI_LOW)
        dist_tick_2 = tick - start_tick_2;
}


void cb_func_echo3(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
        start_tick_3 = tick;
    else if(level == PI_LOW)
        dist_tick_3 = tick - start_tick_3;
}


void setgpiomode(int pi,int range)
{   
    set_mode(pi, INPUT1, PI_OUTPUT); set_mode(pi, INPUT2, PI_OUTPUT);
    set_mode(pi, INPUT3, PI_OUTPUT); set_mode(pi, INPUT4, PI_OUTPUT);
    set_mode(pi, go_pin, PI_OUTPUT); set_mode(pi, back_pin, PI_OUTPUT);
    set_mode(pi, left_pin, PI_OUTPUT); set_mode(pi, right_pin, PI_OUTPUT);
    set_mode(pi, auto_pin, PI_OUTPUT);
    set_PWM_range(pi,INPUT1,range);    set_PWM_range(pi,INPUT2,range);
    set_PWM_range(pi,INPUT3,range);    set_PWM_range(pi,INPUT4,range);
    set_mode(pi, TRIG_PINNO1, PI_OUTPUT);
    set_mode(pi, ECHO_PINNO1, PI_INPUT);
    set_mode(pi, TRIG_PINNO2, PI_OUTPUT);
    set_mode(pi, ECHO_PINNO2, PI_INPUT); 
    set_mode(pi, TRIG_PINNO3, PI_OUTPUT);
    set_mode(pi, ECHO_PINNO3, PI_INPUT);
    set_pull_up_down(pi, 7, PI_PUD_UP);
    gpio_write(pi, TRIG_PINNO1, PI_OFF);
    gpio_write(pi, TRIG_PINNO2, PI_OFF);
    gpio_write(pi, TRIG_PINNO3, PI_OFF);
}

int left_encor(int pi, int range)
{
    stop_pwm(pi);
    num3 = 0;
    num4 = 0;
    left_pwm(pi, range);
    while(1)
    {
    if(num3 > 7)
    {
        stop_pwm(pi);
        num3 = 0;
        num4 = 0;
        go_pwm(pi, range);
        time_sleep(0.1);
        break;
    }
    }
    key = 6;
}

int right_encor(int pi, int range)
{
    stop_pwm(pi);
    num3 = 0;
    num4 = 0;
    right_pwm(pi, range);
    while(1)
    {
        if(num3 > 7)
        {
            stop_pwm(pi);
            num3 = 0;
            num4 = 0;
            go_pwm(pi, range);
            time_sleep(0.1);
            break;
        }
    }
    key = 6;
}

int turn_encor(int pi, int range)
{
    stop_pwm(pi);
    num3 = 0;
    num4 = 0;
    right_pwm(pi, range);
    while(1)
    {
        if(num3 > 17)
        {
            stop_pwm(pi);
            num3 = 0;
            num4 = 0;
            go_pwm(pi, range);
            time_sleep(0.1);
            break;
        }
    }
    key = 6;
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

void cb_func_encor1(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
        start_encor1 = tick;
    else if(level == PI_LOW)
        dist_encor1 = tick - start_encor1;
    num3++;
}

void cb_func_encor2(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
        start_encor2 = tick;
    else if(level == PI_LOW)
        dist_encor2 = tick - start_encor2;
    num4++;
}

void *thr_fn1(void *arg)
{
    int pi = 1;

    if((pi = pigpio_start(NULL, NULL)) < 0)
    {
        fprintf(stderr, "pigpio_start error\n");
    }
    printf("실행중\n");
    setgpiomode(pi, range);
    set_mode(pi, DO_L, PI_INPUT);
    set_mode(pi, DO_R, PI_INPUT);
    callback(pi, DO_L, EITHER_EDGE, cb_func_encor1);
    callback(pi, DO_R, EITHER_EDGE, cb_func_encor2);
    while(1){
        camera_updown(pi);
        camera_leftright(pi);
        while(key == 1)
        {
            go_pwm(pi,range);
        camera_updown(pi);
        camera_leftright(pi);
 
            if(camera == 1)
            {
                camera_turn_left(pi);
                camera = 0;
            }
            if(camera == 2)
            {
                camera_turn_right(pi);
                camera = 0;
            }
            if(camera == 3)
            {
                camera_up(pi);
                camera = 0;
            }
            if(camera == 4)
            {
                camera_down(pi);
                camera = 0;
            }


            while(40 < num3)
            {
                gpio_write(pi, INPUT1, PI_LOW);
                gpio_write(pi, INPUT4, PI_LOW);
                set_PWM_dutycycle(pi, INPUT2, (range*wgt2/1000));
                gpio_write(pi, INPUT3, PI_LOW);
                if(41 < num4)
                {
                    num3 = 1;
                    num4 = 0;
                    break;
                }
            }
            while(40 < num4)
            {
                gpio_write(pi, INPUT1, PI_LOW);
                gpio_write(pi, INPUT4, PI_LOW);
                set_PWM_dutycycle(pi, INPUT3, (range*wgt1/1000));
                gpio_write(pi, INPUT2, PI_LOW);
                if(41 < num3)
                {
                    num3 = 0;
                    num4 = 1;
                    break;
                }
            }


        }
        while(key == 2)
        {
        camera_updown(pi);
        camera_leftright(pi);
 
            back_pwm(pi,range);
            if(camera == 1)
            {
                camera_turn_left(pi);
                camera = 0;
            }
            if(camera == 2)
            {
                camera_turn_right(pi);
                camera = 0;
            }
            if(camera == 3)
            {
                camera_up(pi);
                camera = 0;
            }
            if(camera == 4)
            {
                camera_down(pi);
                camera = 0;
            }
            while(40 < num3)
            {
                gpio_write(pi, INPUT2, PI_LOW);
                gpio_write(pi, INPUT4, PI_LOW);
                set_PWM_dutycycle(pi, INPUT1, (range*wgt2/1000));
                gpio_write(pi, INPUT3, PI_LOW);
                if(41 < num4)
                {
                    num3 = 1;
                    num4 = 0;
                    break;
                }
            }
            while(40 < num4)
            {
                gpio_write(pi, INPUT1, PI_LOW);
                gpio_write(pi, INPUT3, PI_LOW);
                set_PWM_dutycycle(pi, INPUT4, (range*wgt1/1000));
                gpio_write(pi, INPUT2, PI_LOW);
                if(41 < num3)
                {
                    num3 = 0;
                    num4 = 1;
                    break;
                }
            }


        } 
        while(key == 3)
        {
        camera_updown(pi);
        camera_leftright(pi);
 
            left_pwm(pi, range);
            if(camera == 1)
            {
                camera_turn_left(pi);
                camera = 0;
            }
            if(camera == 2)
            {
                camera_turn_right(pi);
                camera = 0;
            }
            if(camera == 3)
            {
                camera_up(pi);
                camera = 0;
            }
            if(camera == 4)
            {
                camera_down(pi);
                camera = 0;
            }
            break;
        }
        while(key == 4)
        {
        camera_updown(pi);
        camera_leftright(pi);
 
            right_pwm(pi, range);
            if(camera == 1)
            {
                camera_turn_left(pi);
                camera = 0;
            }
            if(camera == 2)
            {
                camera_turn_right(pi);
                camera = 0;
            }
            if(camera == 3)
            {
                camera_up(pi);
                camera = 0;
            }
            if(camera == 4)
            {
                camera_down(pi);
                camera = 0;
            }
            break;
        }
        while(key == 5)
        {
        camera_updown(pi);
        camera_leftright(pi);
 
            stop_pwm(pi);
            if(camera == 1)
            {
                camera_turn_left(pi);
                camera = 0;
            }
            if(camera == 2)
            {
                camera_turn_right(pi);
                camera = 0;
            }
            if(camera == 3)
            {
                camera_up(pi);
                camera = 0;
            }
            if(camera == 4)
            {
                camera_down(pi);
                camera = 0;
            }
            break;
        }
        while(key == 6)
        {
            auto_ctrl(pi,range);
            break;
        }
        if(key > 11)
            break;
    }

}

void camera_turn_right(int pi)
{
    if(pulse1<2450)
        pulse1 += 100;
    set_servo_pulsewidth(pi, camera_right_left_servo, pulse1);
    time_sleep(0.02);
}
void camera_turn_left(int pi)
{
    if(pulse1>550)
        pulse1 -= 100;
    set_servo_pulsewidth(pi, camera_right_left_servo, pulse1);
    time_sleep(0.02);
}
void camera_up(int pi)
{
    if(pulse2<2450)
        pulse2+=100;
    set_servo_pulsewidth(pi, camera_up_down_servo, pulse2);
    time_sleep(0.02);
}

void camera_down(int pi)
{
    if(pulse2 > 550)
        pulse2 -= 100;
    set_servo_pulsewidth(pi, camera_up_down_servo, pulse2);
    time_sleep(0.02);
}

void camera_updown(int pi)
{
    while(camera == 5)
    {
        pulse2 += 5;
        set_servo_pulsewidth(pi, camera_up_down_servo, pulse2);
        time_sleep(0.02);
        if(pulse2 > 2450)
        {
            while(camera == 5)
            {
                pulse2 -= 5;
                set_servo_pulsewidth(pi, camera_up_down_servo, pulse2);
                time_sleep(0.02);
                if(pulse2 < 550)
                    break;
            }
        }
    }
}

void camera_leftright(int pi)
{
    while(camera == 6)
    {
        pulse1 += 5;
        set_servo_pulsewidth(pi, camera_right_left_servo, pulse1);
        time_sleep(0.02);
        if(pulse1 > 2450)
        {
            while(camera == 6)
            {
                pulse1 -= 5;
                set_servo_pulsewidth(pi, camera_right_left_servo, pulse1);
                time_sleep(0.02);
                if(pulse1 < 550)
                    break;
            }
        }
    }
}

/*void human(void)
{
    if(!bcm2835_init()){
        printf("Please run this with sudo\n");
    }

    bcm2835_gpio_fsel(SENSOR, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(LED, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_set_pud(SENSOR, BCM2835_GPIO_PUD_UP);
    uint8_t state = ACTIVE_VALUE;

    while (1) {
        state = bcm2835_gpio_lev(SENSOR); //HIGH or LOW?
        if(state != ACTIVE_VALUE)
        {
            //Sensor not active
        }
        else
        {
            break;
        }
    }
}
#undef SENSOR
#undef LED
#undef ACTIVE_VALUE
*/
void cb_func_go(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
    {
        key = 1;
        printf("<GO>\n");
    }
    else if(level == PI_LOW)
    {
        key = 5;
        num3 = 0;
        num4 = 0;
    }
}

void cb_func_back(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
    {
        key = 2;
        back_pwm(pi,range);
        printf("<BACK>\n");
    }
    else if(level == PI_LOW)
    {
        key = 5;
        num3 = 0;
        num4 = 0;
    }
}

void cb_func_left(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
    {
        key = 3;
        left_pwm(pi, range);
        printf("<LEFT>\n");
    }
    else if(level == PI_LOW)
    {
        key = 5;
        num3 = 0;
        num4 = 0;
    }
}

void cb_func_right(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
    {
        key = 4;
        right_pwm(pi, range);
        printf("<RIGHT>\n");
    }
    else if(level == PI_LOW)
    {
        key = 5;
        num3 = 0;
        num4 = 0;
    }
}

void cb_func_auto(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
    {
        key = 6;
        printf("<AUTO>\n");
    }
    else if(level == PI_LOW)
    {
        num3 = 0;
        num4 = 0;
    }
}

void cb_func_updown(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
    {
        camera = 5;
        printf("camera up, down\n");
    }
    else if(level == PI_LOW)
    {
        camera = 0;
    }
}

void cb_func_leftright(int pi, unsigned gpio, unsigned level, uint32_t tick)
{
    if(level == PI_HIGH)
    {
        printf("camera left, right\n");
        camera = 6;
    }
    else if(level == PI_LOW)
    {
        camera = 0;
    }
}
