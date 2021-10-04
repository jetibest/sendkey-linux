#ifndef XKB_KEYCODES
#define XKB_KEYCODES

unsigned char xkb_keycodes[256] = {0};

// load standard evdev/xfree86 keycode mapping
void xkb_keycodes_init()
{
    xkb_keycodes[49]  =  53; // TLDE/AE00
    xkb_keycodes[10]  =  30; // AE01
    xkb_keycodes[11]  =  31; // AE02
    xkb_keycodes[12]  =  32; // AE03
    xkb_keycodes[13]  =  33; // AE04
    xkb_keycodes[14]  =  34; // AE05
    xkb_keycodes[15]  =  35; // AE06
    xkb_keycodes[16]  =  36; // AE07
    xkb_keycodes[17]  =  37; // AE08
    xkb_keycodes[18]  =  38; // AE09
    xkb_keycodes[19]  =  39; // AE10
    xkb_keycodes[20]  =  45; // AE11
    xkb_keycodes[21]  =  46; // AE12
    xkb_keycodes[22]  =  42; // BKSP

    xkb_keycodes[23]  =  43; // TAB
    xkb_keycodes[24]  =  20; // AD01
    xkb_keycodes[25]  =  26; // AD02
    xkb_keycodes[26]  =   8; // AD03
    xkb_keycodes[27]  =  21; // AD04
    xkb_keycodes[28]  =  23; // AD05
    xkb_keycodes[29]  =  28; // AD06
    xkb_keycodes[30]  =  24; // AD07
    xkb_keycodes[31]  =  12; // AD08
    xkb_keycodes[32]  =  18; // AD09
    xkb_keycodes[33]  =  19; // AD10
    xkb_keycodes[34]  =  47; // AD11
    xkb_keycodes[35]  =  48; // AD12
    xkb_keycodes[36]  =  40; // RTRN

    xkb_keycodes[66]  =  57; // CAPS
    xkb_keycodes[38]  =   4; // AC01
    xkb_keycodes[39]  =  22; // AC02
    xkb_keycodes[40]  =   7; // AC03
    xkb_keycodes[41]  =   9; // AC04
    xkb_keycodes[42]  =  10; // AC05
    xkb_keycodes[43]  =  11; // AC06
    xkb_keycodes[44]  =  13; // AC07
    xkb_keycodes[45]  =  14; // AC08
    xkb_keycodes[46]  =  15; // AC09
    xkb_keycodes[47]  =  51; // AC10
    xkb_keycodes[48]  =  52; // AC11

    xkb_keycodes[50]  = 225; // LFSH
    xkb_keycodes[51]  =  49; // BKSL/AC12
    xkb_keycodes[52]  =  29; // AB01
    xkb_keycodes[53]  =  27; // AB02
    xkb_keycodes[54]  =   6; // AB03
    xkb_keycodes[55]  =  25; // AB04
    xkb_keycodes[56]  =   5; // AB05
    xkb_keycodes[57]  =  17; // AB06
    xkb_keycodes[58]  =  16; // AB07
    xkb_keycodes[59]  =  54; // AB08
    xkb_keycodes[60]  =  55; // AB09
    xkb_keycodes[61]  =  56; // AB10
    xkb_keycodes[62]  = 229; // RTSH

    xkb_keycodes[64]  = 226; // LALT
    xkb_keycodes[37]  = 224; // LCTL
    xkb_keycodes[65]  =  44; // SPCE
    xkb_keycodes[109] = 228; // RCTL
    xkb_keycodes[113] = 230; // RALT
    xkb_keycodes[115] = 227; // LWIN
    xkb_keycodes[116] = 231; // RWIN
    xkb_keycodes[117] = 101; // MENU

    xkb_keycodes[9]   =  41; // ESC
    xkb_keycodes[67]  =  58; // FK01
    xkb_keycodes[68]  =  59; // FK02
    xkb_keycodes[69]  =  60; // FK03
    xkb_keycodes[70]  =  61; // FK04
    xkb_keycodes[71]  =  62; // FK05
    xkb_keycodes[72]  =  63; // FK06
    xkb_keycodes[73]  =  64; // FK07
    xkb_keycodes[74]  =  65; // FK08
    xkb_keycodes[75]  =  66; // FK09
    xkb_keycodes[76]  =  67; // FK10
    xkb_keycodes[95]  =  68; // FK11
    xkb_keycodes[96]  =  69; // FK12

    xkb_keycodes[111] =  70; // PRSC
    xkb_keycodes[92]  = 154; // SYRQ
    xkb_keycodes[78]  =  71; // SCLK
    xkb_keycodes[110] =  72; // PAUS
    xkb_keycodes[114] =  72; //  BRK

    xkb_keycodes[106] =  73; //  INS
    xkb_keycodes[97]  =  74; // HOME
    xkb_keycodes[99]  =  75; // PGUP
    xkb_keycodes[107] =  76; // DELE
    xkb_keycodes[103] =  77; //  END
    xkb_keycodes[105] =  78; // PGDN

    xkb_keycodes[98]  =  82; //  UP
    xkb_keycodes[100] =  80; // LEFT
    xkb_keycodes[104] =  81; // DOWN
    xkb_keycodes[102] =  79; // RGHT

    xkb_keycodes[77]  =  83; // NMLK
    xkb_keycodes[112] =  84; // KPDV
    xkb_keycodes[63]  =  85; // KPMU
    xkb_keycodes[82]  =  86; // KPSU

    xkb_keycodes[79]  =  95; //  KP7
    xkb_keycodes[80]  =  96; //  KP8
    xkb_keycodes[81]  =  97; //  KP9
    xkb_keycodes[86]  =  87; // KPAD

    xkb_keycodes[83]  =  92; //  KP4
    xkb_keycodes[84]  =  93; //  KP5
    xkb_keycodes[85]  =  94; //  KP6

    xkb_keycodes[87]  =  89; //  KP1
    xkb_keycodes[88]  =  90; //  KP2
    xkb_keycodes[89]  =  91; //  KP3
    xkb_keycodes[108] =  88; // KPEN

    xkb_keycodes[90]  =  98; //  KP0
    xkb_keycodes[91]  =  99; // KPDL
    xkb_keycodes[126] = 134; // KPEQ

    xkb_keycodes[118] = 104; // FK13
    xkb_keycodes[119] = 105; // FK14
    xkb_keycodes[120] = 106; // FK15
    xkb_keycodes[121] = 107; // FK16
    xkb_keycodes[122] = 108; // FK17
    xkb_keycodes[123] = 179; // KPDC DC for decimal separator?
}

#endif /* XKB_KEYCODES */
