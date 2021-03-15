#include <kipr/botball.h>

// servos
#define claw_servo 0

// motors
#define power_motor 1
#define track_motor_move 0
#define track_motor_up 3

// sensors
#define track_max_sensor 9
#define track_min_sensor 2
#define light_sensor 0

// position (TO TEST!)
#define small_sky_to_low -7300
#define max_to_low -10300
#define min_to_small_sky 9600
#define max_to_high -1400
#define max_to_small_sky -5000
#define claw_close_position 0
#define claw_open_position 2047

// distance (TO TEST!)
#define between_sky 360
#define drive_till_para_reverse_to_sky 190
#define line_tracking_to_sky 270
#define black_line_to_water 210
#define black_line_to_gas 100

// time (TO TEST!)
#define line_tracking_to_DRZ 800
#define large_sky_to_DRZ 1500
#define drive_out_of_DRZ 1200

// functions
void adjust_with_pipe();
void drive_between_sky(int line_tracking);
void drive_distance(int speed, int distance);
void drive_till_black();
void drive_till_para();
void drive_till_para_reverse();
void drive_till_pipe();
void close_claw();
void connect_create();
void get_battery();
int get_fire_location();
void move_position(int port, int speed, int position);
void open_claw();
void spin(int angle, int direction); // More accurate
void spin_CCW_angle(int speed, int angle); // Less accurate
void spin_CCW_90(); // TO TEST!
void spin_CW_angle(int speed, int angle); // Less accurate
void spin_CW_90(); // TO TEST!
void spin_180(); // TO TEST!
void track_move_max(int stop);
void track_move_min();
void track_up();
void wait();

void good_luck();
// Steps
int main();
void start();
int get_fire_building();
void get_water_container();
void place_water(int building);
void pick_first();
void pick_second();
void pick_third();
void go_to_power(int track_move);
void place_gas();
void connect_power();

/****************************************************************************************************************************************************************/

void good_luck()
{
    void fun1()
    {
        msleep(8000);
        track_up();
        wait();
    }
    thread_start(thread_create(fun1));
    
    int fire_building = get_fire_building();
    wait();
    
    spin(105, -1);
    wait();
    
    adjust_with_pipe();
    wait();
    
    create_drive_straight(100);
    msleep(500);
    create_stop();
    wait();
    
    spin_CW_90();
    wait();
    
    get_water_container();
    wait();
    
    place_water(fire_building);
    wait();
}

int main()
{
    start();
    
    //wait_for_light(light_sensor);
    
    while(1)
    {
        if(analog(0) < 300)
        {
            break;
        }
    }
    
    shut_down_in(119);
    
    good_luck();
    
    return 0;
}

void start()
{
    connect_create();
    create_full();
    /*
     set_servo_position(ve_servo, ve_init_position);
     wait();
     set_servo_position(ho_servo, ho_init_position);
     wait();
     */
    set_servo_position(claw_servo, claw_open_position);
    wait();
    enable_servos();
    wait();
}

int get_fire_building()
{
    int duration = 0;
    int fire_building = 1;
    printf("Start receiving signal...\n");
    while(duration < 15000)
    {
        if((get_create_rbump()==1) || (get_create_lbump()==1))
        {
            while((get_create_rbump()==1) || (get_create_lbump()==1))
            {
                continue;
            }
            fire_building += 1;
            msleep(500);
        }
        msleep(100);
        duration += 100;
    }
    printf("The building on fire is: %d\n", fire_building);
    return fire_building;
}

void get_water_container()
{    
    // track moves up a little bit
    void fun1()
    {
        mav(track_motor_move, 1500);
        msleep(1000);
        freeze(track_motor_move);
    }
    thread_start(thread_create(fun1));
    // end
    
    // get water
    drive_till_para_reverse();
    wait();
    
    drive_distance(-100, black_line_to_water);
    wait();
    
    spin(25, -1);
    wait();
    
    track_move_min();
    wait();
    msleep(500);
    
    close_claw();
    wait();
    
    // track moves up a little bit
    void fun2()
    {
        mav(track_motor_move, 1500);
        msleep(1500);
        freeze(track_motor_move);
    }
    thread_start(thread_create(fun2)); // end
    // end
    
    // move back
    spin(25, 1);
    wait();
    
    drive_till_para();
    wait();
    // end
}

void place_water(int building)
{
    if ((building == 1) || (building == 3))
    {
        // thread: track moves up to small sky
        void fun1()
        {
            move_position(track_motor_move, 1500, min_to_small_sky);
        }
        thread_start(thread_create(fun1));
        // end
    }
    else
    {
        // thread: track moves up to max
        void fun1()
        {
            track_move_max(1);
        }
        thread_start(thread_create(fun1));
        // end
    }
    
    if (building == 1)
    {
        create_drive_straight(100);
        msleep(500);
        create_stop();
        wait();
    }
    if (building != 1)
    {
        drive_between_sky(0);
        wait();
    }
    if (building == 3)
    {
        drive_till_para();
        wait();
        
        create_drive_straight(100);
        msleep(500);
        create_stop();
        wait();
    }
    
    // move to central line
    spin_CW_90();
    wait();
    
    if (building == 1 || building == 2)
    {
        msleep(4000);
    }
    
    if (building == 2)
    {
        drive_till_para_reverse();
        wait();
        
        create_drive_straight(-300);
        msleep(drive_out_of_DRZ / 2);
        wait();
    }
    else
    {
        create_drive_straight(-300);
        msleep(drive_out_of_DRZ);
        wait();
    }
    
    drive_till_para_reverse();
    wait();
    // end
    
    // drop to sky
    drive_distance(-100, drive_till_para_reverse_to_sky - 25);
    wait();
    
    mav(track_motor_move, -1500);
    
    msleep(2000);
    
    freeze(track_motor_move);
}

void pick_first() // right small sky on fire
{
    // thread: track moves down to small sky
    void fun1()
    {
        mav(track_motor_move, -1500);
        msleep(1200);
        freeze(track_motor_move);
    }
    thread_start(thread_create(fun1));
    // end
    
    // move to left small sky
    spin_CW_90();
    wait();
    
    drive_between_sky(1);
    wait();
    
    drive_between_sky(1);
    wait();
    
    spin_CCW_90();
    wait();
    // end
    
    // pick from left small sky
    drive_distance(-100, line_tracking_to_sky);
    wait();
    
    close_claw();
    wait();
    
    mav(track_motor_move, 1500);
    msleep(2500);
    freeze(track_motor_move);
    // end
    
    // thread: track moves down to low
    void fun2()
    {
        msleep(2000);
        move_position(track_motor_move, 1500, small_sky_to_low);
    }
    thread_start(thread_create(fun2));
    // end
    
    // move to pipe, move through DRZ, and drop
    drive_till_pipe();
    wait();
    
    spin_CCW_90();
    wait();
    
    create_drive_straight(-300);
    msleep(500);
    create_stop();
    wait();
    
    drive_till_para();
    wait();
    
    drive_between_sky(0);
    wait();
    // end
    
    // move to central line
    spin_CCW_90();
    wait();
    
    drive_till_para();
    wait();
    
    create_drive_straight(200);
    msleep(1000);
    create_stop();
    wait();
    
    open_claw();
    wait();
    
    // thread: track moves up to high
    void fun3()
    {
        track_move_max(1);
        wait();
        
        move_position(track_motor_move, 1500, max_to_high);
        wait();
    }
    thread_start(thread_create(fun3));
    // end
    
    spin_180();
    wait();
    
    drive_till_para_reverse();
    wait();
    // end
    
    // pick from large sky
    msleep(3000);
    
    drive_distance(-100, drive_till_para_reverse_to_sky);
    wait();
    
    close_claw();
    wait();
    
    track_move_max(0);
    wait();
    // end
    
    // drop to DRZ
    create_drive_straight(300);
    msleep(large_sky_to_DRZ);
    wait();
    
    spin_180();
    wait();
    
    freeze(track_motor_move);
    wait();
    
    move_position(track_motor_move, 1500, max_to_low);
    wait();
    
    open_claw();
    wait();
    
    drive_till_para();
    wait();
    
    create_drive_straight(100);
    msleep(500);
    create_stop();
    wait();
    // end
}

void pick_second() // central large sky on fire
{
    // thread: track moves down to small sky
    void fun1()
    {
        move_position(track_motor_move, 1500, max_to_small_sky);
        wait();
    }
    thread_start(thread_create(fun1));
    // end
    
    // move to left small sky
    spin_CW_90();
    wait();
    
    drive_between_sky(1);
    wait();
    
    spin_CCW_90();
    wait();
    // end
    
    // pick from left small sky
    drive_distance(-100, line_tracking_to_sky);
    wait();
    
    close_claw();
    wait();
    
    mav(track_motor_move, 1500);
    msleep(2500);
    freeze(track_motor_move);
    // end
    
    // thread: track moves down to low
    void fun2()
    {
        msleep(2000);
        move_position(track_motor_move, 1500, small_sky_to_low);
    }
    thread_start(thread_create(fun2));
    // end
    
    // move to pipe, move through DRZ, and drop
    drive_till_pipe();
    wait();
    
    spin_CCW_90();
    wait();
    
    drive_between_sky(0);
    wait();
    
    drive_till_para();
    wait();
    
    msleep(1000);
    
    open_claw();
    wait();
    
    // thread: track moves up to small sky
    void fun3()
    {
        track_move_min();
        move_position(track_motor_move, 1500, min_to_small_sky);
    }
    thread_start(thread_create(fun3));
    // end
    
    create_drive_straight(100);
    msleep(500);
    create_stop();
    wait();
    // end
    
    // move to right small sky
    spin_CW_90();
    wait();
    
    msleep(5000);
    
    create_drive_straight(-300);
    msleep(drive_out_of_DRZ);
    wait();
    
    drive_till_para_reverse();
    wait();
    // end
    
    // pick from right small sky
    drive_distance(-100, drive_till_para_reverse_to_sky);
    wait();
    
    close_claw();
    wait();
    
    mav(track_motor_move, 1500);
    msleep(2500);
    freeze(track_motor_move);
    // end
    
    // thread: track moves down to low
    void fun4()
    {
        move_position(track_motor_move, 1500, small_sky_to_low);
    }
    thread_start(thread_create(fun4));
    // end
    
    // move to pipe, and drop
    drive_till_pipe();
    wait();
    
    spin_CCW_90();
    wait();
    
    msleep(1500);
    
    open_claw();
    wait();
    
    spin_CCW_90();
    wait();
    
    create_drive_straight(300);
    msleep(drive_out_of_DRZ);
    wait();
    
    drive_till_para();
    wait();
    
    create_drive_straight(100);
    msleep(500);
    create_stop();
    wait();
    // end
}

void pick_third() // the left small sky on fire
{
    // thread: track moves up to high
    void fun1()
    {
        track_move_max(1);
        wait();
        
        move_position(track_motor_move, 1500, max_to_high);
        wait();
    }
    thread_start(thread_create(fun1));
    // end
    
    // move to large sky
    spin_CCW_90();
    wait();
    
    drive_between_sky(1);
    wait();
    
    spin_CW_90();
    wait();
    // end
    
    // pick from large sky
    drive_distance(-100, line_tracking_to_sky);
    wait();
    
    close_claw();
    wait();
    
    track_move_max(0);
    wait();
    // end
    
    // drop to DRZ
    create_drive_straight(300);
    msleep(large_sky_to_DRZ);
    
    spin_180();
    wait();
    
    freeze(track_motor_move);
    wait();
    
    move_position(track_motor_move, 1500, max_to_low);
    wait();
    
    open_claw();
    wait();
    
    drive_till_para();
    wait();
    
    // thread: track moves up to small sky
    void fun3()
    {
        track_move_min();
        wait();
        
        move_position(track_motor_move, 1500, min_to_small_sky);
    }
    thread_start(thread_create(fun3));
    // end
    
    create_drive_straight(100);
    msleep(500);
    create_stop();
    wait();
    // end
    
    //move to right small sky
    spin_CW_90();
    wait();
    
    drive_between_sky(1);
    wait();
    
    spin_CW_90();
    wait();
    // end
    
    // pick from right small sky
    drive_distance(-100, line_tracking_to_sky);
    wait();
    
    close_claw();
    wait();
    
    mav(track_motor_move, 1500);
    msleep(2500);
    freeze(track_motor_move);
    // end
    
    // thread: track moves down to low
    void fun4()
    {
        move_position(track_motor_move, 1500, small_sky_to_low);
    }
    thread_start(thread_create(fun4));
    // end
    
    // move to pipe, and drop
    drive_till_pipe();
    wait();
    
    spin_CCW_90();
    wait();
    
    msleep(1500);
    
    open_claw();
    wait();
    
    spin_CCW_90();
    wait();
    
    create_drive_straight(300);
    msleep(drive_out_of_DRZ);
    wait();
    
    drive_till_para();
    wait();
    
    create_drive_straight(100);
    msleep(500);
    create_stop();
    wait();
    // end
}

void go_to_power(int move_track)
{   
    spin_CW_90();
    wait();
    
    if (move_track)
    {
        // thread: track moves down to min
        void fun1()
        {
            track_move_min();
        }
        thread_start(thread_create(fun1));
        // end
    }
    
    while((get_create_rbump()==0) && (get_create_lbump()==0))
    {
        create_drive_direct(500-250*((get_create_rfcliff_amt()-1000)/1650), 500-250*((get_create_lfcliff_amt()-1000)/1650));
    }
    create_drive_straight(-100);
    msleep(500);
    create_stop();
}

void place_gas()
{
    // move to gas, and pick
    drive_distance(-200, black_line_to_gas);
    wait();
    
    spin_CW_90();
    wait();
    
    drive_distance(-200, 150);
    wait();
    
    close_claw();
    wait();
    
    // thread: track moves up
    void fun1()
    {
        mav(track_motor_move, 1500);
    }
    thread_start(thread_create(fun1));
    // end
    
    msleep(3000);
    // end
    
    // move to UZ, and drop
    create_drive_straight(300);
    msleep(1000);
    create_stop();
    wait();
    
    while (get_create_lfcliff_amt() > 2650 && get_create_rfcliff_amt() > 2650)
    {
        create_drive_straight(200);
    }
    create_stop();
    wait();
    
    spin(150, 1);
    wait();
    
    freeze(track_motor_move);
    track_move_min();
    wait();
    
    open_claw();
    wait();
    
    mav(track_motor_move, 1500);
    msleep(2000);
    freeze(track_motor_move);
    // end
    
    // move back to central line
    spin(30, 1);
    wait();
    
    drive_till_para();
    wait();
    
    create_drive_straight(100);
    msleep(500);
    create_stop();
    wait();
    // end
}

void connect_power()
{	
    create_drive_straight(-100);
    msleep(500);
    create_stop();
    wait();
    
    spin_CW_90();
    wait();
    
    create_drive_straight(100);
    msleep(300);
    create_stop();
    wait();
    
    move_position(power_motor, 300, 930);
    wait();
    
    create_drive_straight(-100);
    msleep(2000);
    create_stop();
    wait();
    
    drive_till_para();
    wait();
    
    create_drive_straight(100);
    msleep(3000);
    create_stop();
    wait();
    
    int i;
    for (i=0; i<5; i++)
    {
        create_drive_straight(-100);
        msleep(4000);
        create_drive_straight(100);
        msleep(4000);
    }
    create_stop();
}

/****************************************************************************************************************************************************************/

void adjust_with_pipe()
{
    create_drive_straight(-200);
    msleep(800);
    create_stop();
}

void connect_create()
{
    create_connect();
    msleep(1500);
}

void drive_between_sky(int line_tracking)
{
    if (line_tracking)
    {
        set_create_distance(0);
        wait();
        while(get_create_distance() < between_sky)
        {
            create_drive_direct(200-100*( (get_create_rfcliff_amt()-1000)/1650), 200-100*( (get_create_lfcliff_amt()-1000)/1650));
        }
        create_stop();
    }
    else
    {
        set_create_distance(0);
        wait();
        while(get_create_distance() < between_sky)
        {
            create_drive_straight(300);
        }
        create_stop();
    }
    
}

void drive_distance(int speed, int distance)
{
    if(speed > 0)
    {
        set_create_distance(0);
        wait();
        while (get_create_distance() < distance)
        {
            create_drive_straight(speed);
        }
    }
    else
    {
        set_create_distance(0);
        wait();
        while (abs(get_create_distance()) < distance)
        {
            create_drive_straight(speed);
        }
    }
    create_stop();
}

void drive_till_black()
{
    while ((get_create_lfcliff_amt() > 1600) && (get_create_rfcliff_amt() > 1600)){
        create_drive_direct(200, 200);
    }
    while(get_create_lfcliff_amt() > 1600){
        create_drive_direct(200, 0);
    }
    while(get_create_rfcliff_amt() > 1600){
        create_drive_direct(0, 200);
    }
    create_stop();
}

void drive_till_para()
{
    while ((get_create_lcliff_amt() > 1600) && (get_create_rcliff_amt() > 1600)){
        create_drive_direct(200, 200);
    }
    while(get_create_lcliff_amt() > 1600){
        create_drive_direct(200, 0);
    }
    while(get_create_rcliff_amt() > 1600){
        create_drive_direct(0, 200);
    }
    create_stop();
}

void drive_till_para_reverse()
{
    while ((get_create_lcliff_amt() > 1600) && (get_create_rcliff_amt() > 1600)){
        create_drive_direct(-200, -200);
    }
    while(get_create_lcliff_amt() > 1600){
        create_drive_direct(-200, 0);
    }
    while(get_create_rcliff_amt() > 1600){
        create_drive_direct(0, -200);
    }
    create_stop();
}

void drive_till_pipe()
{
    while((get_create_rbump()==0) && (get_create_lbump()==0))
    {
        create_drive_straight(300);
    }
    create_drive_straight(-100);
    wait();
    create_stop();
}

void close_claw()
{
    set_servo_position(claw_servo, claw_close_position);
    msleep(500);
}

void get_battery()
{
    printf("%d\n", get_create_battery_capacity());
    printf("%d\n", get_create_battery_charge());
}

int get_fire_location()
{    
    int i = 0;
    int test;
    int location = -1;
    camera_open();
    while(i < 10)
    {
        camera_update();
        test = get_object_center_x(3,0);
        if(test != -1)
        {
            location = test;
            break;
        }
        i++;
    }
    return location;
}

void move_position(int port, int speed, int position)
{
    if (position > 0)
    {
        int target = get_motor_position_counter(port) + position;
        while(get_motor_position_counter(port) < target){
            mav(port, speed);
        }
        freeze(port);
    }
    if (position < 0)
    {
        int target = get_motor_position_counter(port) + position;
        while(get_motor_position_counter(port) > target){
            mav(port, -speed);
        }
        freeze(port);
    }
}

void open_claw()
{
    set_servo_position(claw_servo, claw_open_position);
    msleep(500);
}

void spin(int angle, int direction)
{
    angle = (double)angle * 1.08;
    set_create_total_angle(0);
    while(abs(get_create_total_angle()) < angle){
        int diff = angle - abs(get_create_total_angle());
        if(diff > 60) {
            create_spin_CW(300 * direction);
        }
        else
        {
            int speed = (double)diff * 3.5 + 20;
            create_spin_CW(speed*direction);
            msleep(10);
        }
    }
    create_stop();
}

void spin_CCW_angle(int speed, int angle)
{
    set_create_total_angle(0);
    while(abs(get_create_total_angle()) < angle)
    {
        create_spin_CCW(speed);
    }
    create_stop();
}

void spin_CCW_90()
{
    spin(90, -1);
}

void spin_CW_angle(int speed, int angle)
{
    set_create_total_angle(0);
    while(abs(get_create_total_angle()) < angle)
    {
        create_spin_CW(speed);
    }
    create_stop();
}

void spin_CW_90()
{
    spin(90, 1);
}

void spin_180()
{
    spin(90, 1);
    wait();
    
    spin(90, 1);
}

void track_move_max(int stop)
{
    if(stop)
    {
        while(digital(track_max_sensor) != 1)
        {
            mav(track_motor_move, 1500);
        }
        freeze(track_motor_move);
    }
    else
    {
        mav(track_motor_move, 1500);
    }
    
}

void track_move_min()
{
    while(digital(track_min_sensor) != 1)
    {
        mav(track_motor_move, -1500);
    }
    freeze(track_motor_move);
}

void track_up()
{
    mav(track_motor_up, 1500);
    msleep(9500);
    freeze(track_motor_up);
}

void wait()
{
    msleep(100);
}
