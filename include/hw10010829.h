enum buttons{POWER,UP,DOWN, START,RINSE,TEMP,SPIN,WORK};

void selftest(int A2,int A1,int A0, int CTRLSW);
void buttonCtrl( int button, int pulse, int duty,int IO0,int IO1,int IO2, int IO3);
void bcdconverter(int value, int *bit3, int *bit2, int *bit1, int *bit0);