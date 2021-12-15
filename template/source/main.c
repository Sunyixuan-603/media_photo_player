#include "includes.h"

int main(int argc, char const *argv[])
{
    /*播放开机动画*/
    system(MPLAY_START_PROGRAM);
    login_menu_interface();
    interface_menu();
    return 0;
}



