//version 2 beta

#include <htc.h>                        // definiciones de registros
#include <string.h>
#define _XTAL_FREQ 6000000  // frecuencia de clock, utilizada por la funcion delay

__CONFIG( FOSC_HS & WDTE_OFF & LVP_OFF & BOREN_ON & WRT_OFF & CPD_OFF & DEBUG_OFF & CP_OFF );

//definiciones tiempos E/S en us
#define DLY_START 1000000 //pausa previa al encendido en us
#define DLY_LCDSEND 700 //tiempo de lectura del lcd en us
#define DLY_STEPM1 2500 //tiempo de energizado de bobinas motor1 en us
#define DLY_STEPM1_2 10000 //tiempo de energizado de bobinas motor1 en us para movimientos cortos
#define DLY_STEPM2 2300 //tiempo de energizado de bobinas motor2 en us
#define DLY_LM317PULSE 10 //tiempo de pulso positivo para el driver LM317 en us
#define DLY_MARTILLO_ON 30 //tiempo de energizado de bobina solenoide en ms
#define DLY_MARTILLO_OFF 10 //en ms
#define DLY_M1_ON 200 //en ms


//cantidad de pasos de movimientos
#define ST_CARRO_INITERROR 500  //cantidad maxima de pasos a recorrer con el carro para encontrar su tope
#define ST_METERHOJA 800                //pasos para introducir hoja
#define ST_RETIRARHOJA 3000             //pasos para retirar hoja
#define ST_MEDIDA_A 25                  //espaciado vetical entre puntos(motor2)
#define ST_MEDIDA_B 6                   //espaciado horizontal entre puntos(m1)
#define ST_MEDIDA_C 8                   //espaciado entre letras (m1)
#define ST_MEDIDA_D 120                 //espaciado vertical entre renglones (desde puntos numero 1) (m2)
#define ST_CARRO_POSINICIAL 400 //pasos desde el tope hasta la primera letra
#define ST_RODILLO_FIX 4                //pasos a dar cuando se cambia la direccion del motor del rodillo
#define ST_CARRO_FIX 4                  //pasos a dar cuando se energiza el motor del carro

//direcciones del carro y rodillo
#define DIR_IZQ 1
#define DIR_DER 0
#define DIR_IN 1
#define DIR_OUT 0

//definiciones puerto A
#define ENABLEM1 RA0

//definiciones puerto B
#define LCD7 RB7
#define LCD6 RB6
#define LCD5 RB5
#define LCD4 RB4
#define LCDE RB3
#define LCDRS RB2
#define KBDDATA RB1
#define KBDCLOCK RB0

//definiciones puerto C
#define STROBE RC7
#define MARTILLO RC6
#define SOUND RC5
#define ACK RC4
#define DIRM2 RC3
#define STEPM2 RC2
#define DIRM1 RC1
#define STEPM1 RC0

//definiciones puerto E
#define SPALANCA RE2
#define SFINCARRO RE1
#define SPAPEL RE0

//variables globales

unsigned char CodExtendido=0, CodBreak=0, ready=0, kbdbeep=1, kbddato=0, kbdmoment=0;   //del teclado
unsigned char Papel=0, letrasrenglon=0;
unsigned char impresiondirecta=0, modopc=0;
unsigned char visorcontletras=0, visoractivo=0; // del visor del buffer
unsigned char Buffer[96];
const unsigned char buflen=96; //buflen: longitud de Buffer
char startb=0, imprb=0, introb=0;               //startb: puntero del inicio de cola impresion buffer - imprb: cantidad de letras en la cola de impresion introb: cantidad de letras guardadas sin mandar a imprimir
const unsigned char texto1[]="     Impresora        Braille";
const unsigned char texto2[]="Recibiendo      Papel...";
const unsigned char texto3[]="Iniciando...    0";
const unsigned char texto4[]="Hecho.";
const unsigned char texto5[]="Papel Perdido   Inicio:continuar";
const unsigned char texto6[]="Expulsando      Hoja...";
const unsigned char texto7[]="ERROR SENSOR DE FIN DE CARRO";
const unsigned char texto8[]="Buffer lleno    Espere unos seg.";
const unsigned char texto9[]="No se introdujo texto.";
const unsigned char texto10[]="Buffer borrado.";
const unsigned char texto11[]="Sumado a la colade impresion.";
const unsigned char texto12[]="La palanca esta en una posicion";
const unsigned char texto13[]="que podria cau_ sar problemas.";
const unsigned char texto14[]=" Error teclado";
const unsigned char texto15[]="Impresion direc_ta activada.";
const unsigned char texto16[]="Impresion direc_ta desactivada.";
const unsigned char texto17[]="Sonido de tecla activado.";
const unsigned char texto18[]="Sonido de tecla desactivado.";
const unsigned char texto19[]="No hay papel.   Detenido.";
//const unsigned char texto20[]="Nada que borrar.";
const unsigned char texto21[]=" -- Modo  PC --";
const unsigned char texto22[]="Desconectado.";
const unsigned char texto25[]="Trabajo         Cancelado.";

//const unsigned char signopregunta[8]={14,17,17,2,4,0,4,0};
const unsigned char simboloenter[8]={0,1,1,5,13,31,12,4};
const unsigned char EscudoA[8]={31,16,18,21,21,21,18,16};
const unsigned char EscudoB[8]={31,11,21,11,21,11,21,11};
const unsigned char EscudoC[8]={26,21,26,21,26,21,15,1};
const unsigned char EscudoD[8]={1,21,25,25,21,1,30,16};
const unsigned char letraenie[8]= {
        0B01101,
        0B10110,
        0B00000,
        0B10110,
        0B11001,
        0B10001,
        0B10001,
        0B10001
};
const unsigned char letraacentoA[8] = {
        0B00011,
        0B00110,
        0B00000,
        0B01110,
        0B00001,
        0B01111,
        0B10001,
        0B01111
};
const unsigned char letraacentoE[8] = {
        0B00011,
        0B00110,
        0B00000,
        0B01110,
        0B10001,
        0B11111,
        0B10000,
        0B01110
};
const unsigned char letraacentoI[8] = {
        0B00011,
        0B00110,
        0B00000,
        0B01100,
        0B00100,
        0B00100,
        0B00100,
        0B01110
};
const unsigned char letraacentoO[8] = {
        0B00011,
        0B00110,
        0B00000,
        0B01110,
        0B10001,
        0B10001,
        0B10001,
        0B01110
};
const unsigned char letraacentoU[8] = {
        0B00011,
        0B00110,
        0B00000,
        0B10001,
        0B10001,
        0B10001,
        0B10011,
        0B01101
};
const unsigned char letradieresisU[8] = {
  
        0B00000,
		0B01010,
        0B00000,
        0B10001,
        0B10001,
        0B10001,
        0B10011,
        0B01101
};


//declaracion de funciones
void lcdsend(char);
void lcdinit(void);
void enviartexto(unsigned char[]);
void lcdwritecg(unsigned char, unsigned char[]);
void lcdlinea(char);
void lcdclear(void);
void lcdcursortype(char, char);
void stepm1(char);
void stepm2(void);
void movemotor(char, unsigned int);
void errorfincarro();
void sound(char);
void imprimirletra(unsigned char);
void martillar(void);
void saltodelinea(void);
void pruebaeeprom(void);
void initvisor(void);
void animacion(void);
void encenderm1(void);
void apagarm1(void);
unsigned char ascii2braille( unsigned char , unsigned char );

//********MAIN********

void main(void) {

OPTION_REG=0b00000111;  //bit7 en 0: activar pullup portb; bit6 en 0: interrupcion en flanco negativo 
PIE1=0;


unsigned short i;

TRISA=0b00111110;  // puerto A (RA0 : INHM2)
TRISB=0b00000011;  // puerto B (LCD7, LCD6, LCD5, LCD4, LCDE, LCDRS, KBDDATA, KBDCLOCK)
TRISC=0b10000000;  // puerto C (STROBE(entrada), MARTILO, SOUND, ACK, DIRM2, STEPM2, DIRM1, STEPM1)
TRISD=0xff;  // puerto D entrada (coneccion LPT)
TRISE=0b00000111;  // puerto E entrada (SPALANCA, SFINCARRO, SPAPEL)
ADCON1=0b00000110;      //deshabilitar todos los conversores A/D
PORTA=PORTB=PORTC=PORTD=0;

//tmr init

TMR1CS=0;
T1CKPS1=0;
T1CKPS0=0; 
TMR1ON=0;
TMR1IF=0;
TMR1IE=1;


__delay_us(DLY_START);
lcdinit();
lcdwritecg(0, letraenie);
lcdwritecg(1, letradieresisU);
lcdwritecg(2, letraacentoU);
lcdwritecg(3, simboloenter);
lcdwritecg(4, EscudoA);
lcdwritecg(5, EscudoB);
lcdwritecg(6, EscudoC);
lcdwritecg(7, EscudoD);


// Mensaje Bienvenida
enviartexto(texto1);    //"     Impresora        Braille"
lcdlinea(0);            //imprimir escudo del colegio
lcdsend(4);
lcdsend(5);
lcdlinea(1);
lcdsend(6);
lcdsend(7);
sound(3);
__delay_ms(2000);

lcdclear();
// reemplazar dibujos del escudo por otros simbolos
lcdwritecg(4, letraacentoA);
lcdwritecg(5, letraacentoE);
lcdwritecg(6, letraacentoI);
lcdwritecg(7, letraacentoO);


if (SPALANCA==0) {              //aviso palanca
        
    enviartexto(texto12);
    __delay_ms(4000);
    enviartexto(texto13);
	__delay_ms(4000);

}

enviartexto(texto3);

// mover rodillo


DIRM2=DIR_OUT;
movemotor(2,20);
__delay_ms(100);
movemotor(2,20);
__delay_ms(100);
lcdsend(0x31); //1
movemotor(2,500);
lcdsend(0x32); //2
DIRM2=DIR_IN;
movemotor(2,500);

__delay_ms(1000); //  delay 1s

// Buscar Fin de Carro

lcdsend(0x33); //3

DIRM1=DIR_IZQ;  //direccion carro: izquierda
encenderm1();
for (i=0;i<ST_CARRO_INITERROR;i++){

        if (SFINCARRO==0) break;
        stepm1(1);      

}


if (i==ST_CARRO_INITERROR) errorfincarro();     //error de fin de carro


//tope encontrado, dar 50 pasos y verificar que se deje de tocar el tope
lcdsend(0x34); //4
DIRM1=DIR_DER;
movemotor(1,50);
if (SFINCARRO==0) errorfincarro();      //error de fin de carro
//dar los pasos que faltan
lcdsend(0x35); //5
movemotor(1,(ST_CARRO_POSINICIAL-50));
apagarm1();
enviartexto(texto4);// "Hecho"
sound(3);
INTF=0; //bajar la bandera de interrupcion
INTE=GIE=1; //habilitar interrupcion en rb0 (KBDCLOCK)
ready=1;

//listo para imprimir

while(1) {


        if      (SPAPEL==0&&Papel==1) { //avisar que el papel se perdio y pausar la impresion
                __delay_ms(1000);
                if      (SPAPEL==0&&Papel==1) {
                        enviartexto(texto5); //papel perdido
                        if (imprb!=0) {
                                introb=introb+imprb;
                                imprb=0;        
                                sound(1);
                        }       
                        impresiondirecta=0;     
                }
        }

        if (Papel==1&&imprb!=0&&SPAPEL==1) {    //imprimir la siguiente letra de la cola de impresion
                if (ENABLEM1==0) encenderm1();
                imprimirletra(Buffer[startb]);
                if (startb<(buflen-1)) {
                        startb++;
                } else {
                        startb=0;
                };

        if (imprb!=0) imprb--;
}

        if (SPAPEL==1&&Papel==1&&impresiondirecta==1) { //modo inmediato
                if (introb!=0) {
                        imprb++;
                        introb--;
                }
        }

        if (modopc==1&&PORTD!=0&&STROBE==1&&SPAPEL==1) {
                i=0;
                while (STROBE==1&&i<10) {
                        __delay_ms(1);
                        i++;
                };
                
                if (STROBE==0) {
                ACK=1;
                if (ENABLEM1==0) encenderm1();

                if (visoractivo==1&&visorcontletras<31) {
                        if (visorcontletras==15) lcdlinea(1);
                        lcdsend(PORTD);
                        visorcontletras++;
                } else {
                        initvisor();
                        lcdsend(PORTD);
                };
                        

                imprimirletra(PORTD);
                if (SPAPEL==0) {
                        enviartexto(texto19);   //avisar de que se quedo sin papel
                };
                ACK=0;
                __delay_ms(1);
                };

        };


if ((impresiondirecta==0&&imprb==0)||(impresiondirecta==1&&introb==0&&imprb==0)||(modopc==1&&PORTD==0)) apagarm1();                     //si no hay nada mas que imprimir, desenergizar motor

}

} //*******FIN DE MAIN**********************************************



//RUTINA DE INTERRUPCION


interrupt interrupcion (void) {


unsigned char letra=0, kbdpow=0;
unsigned long i;
unsigned char j;



if (TMR1IF==1) {	//desborde del timer

	enviartexto(texto14);
	TMR1ON=0;
	TMR1L=0;
	TMR1H=0;
	TMR1IF=0;
	kbdmoment=0;
	INTF=0;
	return;	

}


//interrupcion por teclado:

switch (kbdmoment) {
	case 0:
			TMR1ON=1;
			kbddato=0;
			kbdmoment++;
			if (kbdbeep==1) SOUND=1;	//sonido tecla
			INTF=0;
			return;
	case 1: if (kbdpow==0) kbdpow=1;
	case 2: if (kbdpow==0) kbdpow=2;
	case 3: if (kbdpow==0) kbdpow=4;
	case 4: if (kbdpow==0) kbdpow=8;
	case 5: if (kbdpow==0) kbdpow=16;
	case 6: if (kbdpow==0) kbdpow=32;
	case 7: if (kbdpow==0) kbdpow=64;
	case 8: if (kbdpow==0) kbdpow=128;
			
			kbddato =  kbddato + ( KBDDATA * kbdpow );
			kbdmoment++;
			INTF=0;
			return;
	case 9:
			kbdmoment++;
			INTF=0;
			return;

}


	//kbdmoment es 10.

TMR1ON=0;
TMR1L=0;
TMR1H=0;
kbdmoment=0;


		//hacer lo correspondiente segun el mensaje del teclado recibido:


if (ready!=1) {
        INTF=0;         //borrar flag de interrupcion
        return;
}

if (kbddato==0xE0) {
        CodExtendido=1;
        INTF=0;         //borrar flag de interrupcion
        return;
};

if (kbddato==0xF0) {
        CodBreak=1;
        INTF=0;         //borrar flag de interrupcion
        return;
};

if (CodBreak==0&&CodExtendido==1&&(kbddato==0x75||kbddato==0x72||kbddato==0x6b||kbddato==0x74)){
        switch (kbddato) {
                case 0x75: DIRM2=DIR_OUT;       //flecha arriba
                        stepm2();
                        break;
                case 0x72: DIRM2=DIR_IN;        //flecha abajo
                        stepm2();
                        break;
                case 0x6b: DIRM1=DIR_IZQ;        //flecha izquierda
						ENABLEM1=1;
						__delay_ms(DLY_M1_ON);
                        stepm1(1);
						apagarm1();
                        break;
                case 0x74: DIRM1=DIR_DER;        //flecha derecha
						ENABLEM1=1;
						__delay_ms(DLY_M1_ON);
                        stepm1(1);
						apagarm1();
                        break;

        }       
}


if (CodBreak==0) {      //ignorar pulsaciones
        CodExtendido=0;
        INTF=0;         //borrar flag de interrupcion
        return;
};

SOUND=0;

if (CodExtendido==0) {
switch (kbddato) {

        case 0x1C: letra='A';
                                break;
        case 0x32: letra='B';
                                break;
        case 0x21: letra='C';
                                break;
        case 0x23: letra='D';
                                break;
        case 0x24: letra='E';   
                                break;
        case 0x2b: letra='F';
                                break;
        case 0x34: letra='G';
                                break;
        case 0x33: letra='H';
                                break;
        case 0x43: letra='I';   
                                break;
        case 0x3b: letra='J';
                                break;
        case 0x42: letra='K';
                                break;
        case 0x4b: letra='L';
                                break;
        case 0x3a: letra='M';
                                break;
        case 0x31: letra='N';
                                break;
        case 0x44: letra='O';
                                break;
        case 0x4d: letra='P';
                                break;
        case 0x15: letra='Q';
                                break;
        case 0x2d: letra='R';
                                break;
        case 0x1b: letra='S';
                                break;
        case 0x2c: letra='T';
                                break;
        case 0x3c: letra='U';                                   
                                break;
        case 0x2a: letra='V';
                                break;
        case 0x1d: letra='W';
                                break;
        case 0x22: letra='X';
                                break;
        case 0x35: letra='Y';
                                break;
        case 0x1a: letra='Z';
                                break;
        case 0x69: letra=4;             //á
                                break;
        case 0x72: letra=5;             //é
                                break;
        case 0x7a: letra=6;             //í
                                break;
        case 0x6b: letra=7;             //ó
                                break;
        case 0x73: letra=2;             //ú
                                break;
        case 0x74: letra=1;             //ü
                                break;
        case 0x45: letra='J';
                                break;
        case 0x16: letra='A';
                                break;
        case 0x1e: letra='B';
                                break;
        case 0x26: letra='C';
                                break;
        case 0x25: letra='D';
                                break;
        case 0x2e: letra='E';
                                break;
        case 0x36: letra='F';
                                break;
        case 0x3d: letra='G';
                                break;
        case 0x3e: letra='H';
                                break;
        case 0x46: letra='I';
                                break;
        case 0x29: letra=' ';
                                break;
        case 0x41: letra=',';
                                break;
        case 0x49: letra='.';
                                break;
        case 0x4A: letra='_';
                                break;
        case 0x4E: letra='?';
                                break;
        case 0x4C: letra=8;             //n
                                break;
        case 0x52: letra=';';
                                break;
        case 0x55: letra='!';
                                break;
        case 0x54: letra='(';
                                break;
        case 0x5B: letra=')';
                                break;
        case 0x5D: letra='"';
                                break;
        case 0x0E: letra='*';
                                break;
        case 0x58: letra='^';   //caps lock
                                break;
        case 0x0D: letra='#';   //tab
                                break;
        case 0x5A:              //enter
                                letra=3;
                                break;
        case 0x66: if (introb>0) {
                                        introb--;       //backspace
                                        if (visoractivo==1) {
                                                        if (visorcontletras<16) {
                                                                LCDRS=0;
                                                                lcdsend(0x80+visorcontletras);
                                                                LCDRS=1;
                                                                lcdsend(' ');
                                                                LCDRS=0;
                                                                lcdsend(0x80+visorcontletras);
                                                                LCDRS=1;
                                                        };

                                                        if (visorcontletras>=16) {
                                                                LCDRS=0;
                                                                lcdsend(0xC0+(visorcontletras-16));
                                                                LCDRS=1;
                                                                lcdsend(' ');
                                                                LCDRS=0;
                                                                lcdsend(0xC0+(visorcontletras-16));
                                                                LCDRS=1;
                                                        };      
                                                        visorcontletras--;                              

                                        } else {
                                                initvisor();
                                        };
                                }                       
                                break;
        case 5: //F1
                                if (kbdbeep==1) {
                                        enviartexto(texto18);
                                        kbdbeep=0;
                                } else {
                                        enviartexto(texto17);
                                        kbdbeep=1;
                                };
                                break;
        case 6: //F2
                                if (impresiondirecta==1) {
                                        enviartexto(texto16);
                                        impresiondirecta=0;
                                } else {
                                        if (SPAPEL==1) {
                                                enviartexto(texto15);
                                                impresiondirecta=1;
                                        } else {
                                                enviartexto(texto19);
                                                sound(1);
                                        };
                                };
                                break;
        case 4: //F3
                animacion();
                break;
        case 1: //F9
                lcdclear();
                lcdsend(0x30+RD7);
                lcdsend(0x30+RD6);
                lcdsend(0x30+RD5);
                lcdsend(0x30+RD4);
                lcdsend(0x30+RD3);
                lcdsend(0x30+RD2);
                lcdsend(0x30+RD1);
                lcdsend(0x30+RD0);
                lcdsend(0x20);
                lcdsend(0x20);
                lcdsend(0x20);
                lcdsend('S');
                lcdsend('T');
                lcdsend('B');
                lcdsend(0x30+STROBE);
                lcdlinea(1);
                lcdsend('P');
                lcdsend('P');
                lcdsend(0x30+SPAPEL);
                lcdsend('F');
                lcdsend('C');
                lcdsend(0x30+SFINCARRO);
                lcdsend('P');
                lcdsend('L');
                lcdsend(0x30+SPALANCA);         
                break;
        case 0x0A:           //F8
                break;
        case 0x09: martillar();         //F10
                break;
        case 0x78:                      //F11
                LCDE=1;
                __delay_us(DLY_LCDSEND);
                LCDE=0;
                __delay_us(DLY_LCDSEND);
                break;
        case 0x07:                      //F12
                if (modopc==0&&STROBE==1) {
                        enviartexto(texto21);
                        modopc=1;
                        introb=0;
                        imprb=0;
                        impresiondirecta=0;
                } else {
                        enviartexto(texto22);
                        modopc=0;
                };
                break;
        case 0x03:      //F5
                lcdinit();
                lcdwritecg(0, letraenie);
                lcdwritecg(1, letradieresisU);
                lcdwritecg(2, letraacentoU);
                lcdwritecg(3, simboloenter);
                lcdwritecg(4, letraacentoA);
                lcdwritecg(5, letraacentoE);
                lcdwritecg(6, letraacentoI);
                lcdwritecg(7, letraacentoO);
                break;
        case 0x0B:      //F6
                lcdclear();
                for (j=0;j<8;j++){
                        lcdsend(j);
                        __delay_ms(10);
                };
                break;
        case 0x76:      //ESC
                        startb=0;
                        imprb=0;
                        introb=0;
                        enviartexto(texto25);   //Cancelado.
                break;

}
if (letra!=0&&modopc==0) {
        if((introb+imprb)<buflen) {
                                j=startb+imprb+introb;
                                if (j>=buflen) j=j-buflen;
                                Buffer[j]=letra;//guardar letra
                                introb++;

                                if (visoractivo==1&&visorcontletras<31) {
                                        if (visorcontletras==15) lcdlinea(1);
										if ( ( Buffer[j]<0x7B ) && ( Buffer[j]>0x60) ) {	//pasar de mayuscula a minuscula
											lcdsend(Buffer[j]-0x20);
										} else {
                                        	lcdsend(Buffer[j]);
										}
                                        visorcontletras++;
                                } else {
                                        initvisor();
										if ( ( Buffer[j]<0x7B ) && ( Buffer[j]>0x60) ) {	//pasar de mayuscula a minuscula
											lcdsend(Buffer[j]-0x20);
										} else {
                                        	lcdsend(Buffer[j]);
										}
                                };
                        } else {
                                enviartexto(texto8); //"buffer lleno, espere"
                                sound(1);
                        }
}

}


if (CodExtendido==1) {
        switch (kbddato) {
                case 0x69:      //end
                        enviartexto(texto6); //retirando hoja
                        DIRM2=DIR_IN;
                        for (i=0;i<(ST_RETIRARHOJA/10);i++) {
                                movemotor(2,10);
                                if (SPAPEL==0) break;
                        };
                        movemotor(2,800);
                        Papel=0;
                        if (letrasrenglon!=0) {
                                DIRM2=DIR_IN;
                                movemotor(2,ST_MEDIDA_D);
                                DIRM1=DIR_DER;
                                encenderm1();
                                movemotor(1,(letrasrenglon*(ST_MEDIDA_C+ST_MEDIDA_B)));
                                letrasrenglon=0;
                        };
                        impresiondirecta=0;
                        enviartexto(texto4);  //ok
                        break;
                case 0x6C: //home - inicio
                        if (introb!=0) {
                                        if (SPAPEL==1) {
                                                imprb=imprb+introb;
                                                introb=0;
                                                enviartexto(texto11);   //Enviado a cola.
                                        } else {
                                                enviartexto(texto19);
                                                sound(1);
                                        };
                                } else {
                                        enviartexto(texto9);    //no se introdujo texto
                                };
                        break;  
                case 0x70:      //insert
                        if (SPAPEL==1) {
                                enviartexto(texto2); //recibiendo papel
                                DIRM2=DIR_IN;
                                movemotor(2,ST_METERHOJA);
                                Papel=1;
                                enviartexto(texto4);// "OK"
                        } else {
                                enviartexto(texto19);   //no hay hoja
                        };
                        break;

                case 0x71:       //delete
                                        introb=0;
                                        enviartexto(texto10);   //Buffer borrado.
                        break;
                case 0x7A:      //pagedown
                        if (imprb==0) {
                                DIRM2=DIR_IN;
                                movemotor(2,ST_MEDIDA_D);
                        };
                        break;
                case 0x7D:      //pageup
                        if (imprb==0) {
                                DIRM2=DIR_OUT;
                                movemotor(2,ST_MEDIDA_D); //+ST_RODILLO_FIX);
                                DIRM2=DIR_IN;
                                movemotor(2,ST_RODILLO_FIX);
                        };
                        break;
        }


}

CodExtendido=0;
CodBreak=0;
INTF=0;         //borrar flag de interrupcion
}



                        //******funciones*****

//LCD***



void lcdsend(char lcddata) {    //envia al lcd un caracter de a cuatro bits
unsigned char lcdnibble;

lcdnibble=(lcddata&0xF0);
PORTB=(PORTB&0x0F)+lcdnibble;
__delay_us(1);
LCDE=1;
__delay_us(DLY_LCDSEND);
LCDE=0;
__delay_us(DLY_LCDSEND);
lcdnibble=(lcddata<<4);
PORTB=(PORTB&0x0F)+lcdnibble;
__delay_us(1);
LCDE=1;
__delay_us(DLY_LCDSEND);
LCDE=0;
__delay_us(DLY_LCDSEND);
}


void lcdinit(void) {            //inicializa el lcd
LCDRS=0;
lcdsend(2);
lcdsend(40);
lcdcursortype(0,0);

}

void enviartexto(unsigned char v[]){    //envia un conjunto de caracteres al lcd
unsigned char m=strlen(v),i,cantletras=0;

ready=0;        //evitar que una interrupccion escriba en el lcd en un mal momento (error del sist. 4bits)
//lcdclear:

visoractivo=0;
LCDRS=0;
lcdsend(0x80);  //ir a la linea 1
lcdsend(12); //cursor:ninguno
LCDRS=1;


if (m<16) {
        for (i=0;i<m;i++){
                lcdsend(v[i]);
                cantletras++;
        }
} else {

        for (i=0;i<16;i++){
                lcdsend(v[i]);
                cantletras++;
        }
        lcdlinea(1);
        for (;i<m;i++){
                lcdsend(v[i]);
                cantletras++;
        }
};

if (cantletras<=16) {
        for (i=0;i<(16-cantletras);i++) {
                lcdsend(' ');
        };
        lcdlinea(1);
        for (i=0;i<16;i++) {
                lcdsend(' ');
        };
};
if (cantletras>16) {
        for (i=0;i<(32-cantletras);i++) {
                lcdsend(' ');
        };
};


//poner cursor al final del texto

if (cantletras<=16) {
        LCDRS=0;
        lcdsend(0x80+cantletras);
        LCDRS=1;
};

if (cantletras>16) {
        LCDRS=0;
        lcdsend(0xC0+(cantletras-16));
        LCDRS=1;
};


ready=1;
}

void lcdwritecg(unsigned char pos, unsigned char filas[]){      //guarda un caracter personalizado en el lcd

LCDRS=0;
lcdsend(64+pos*8);
LCDRS=1;

//reciclo la variable pos

for (pos=0;pos<8;pos++) lcdsend(filas[pos]);

LCDRS=0;
lcdsend(128);
LCDRS=1;

}

void lcdlinea(char linea){

LCDRS=0;
if (linea==0) lcdsend(0x80);
if (linea==1) lcdsend(0xC0);
LCDRS=1;
}

void lcdclear(void){
unsigned char i;

LCDRS=0;
lcdsend(0x80);          //lcdlinea(0)
LCDRS=1;
for (i=0;i<16;i++) {
        lcdsend(0x20);
}
LCDRS=0;
lcdsend(0xC0);
LCDRS=1;
for (i=0;i<16;i++) {
        lcdsend(0x20);
}
LCDRS=0;
lcdsend(0x80);
LCDRS=1;
}

void lcdcursortype(char cursor, char parpadeo){
LCDRS=0;
lcdsend(12+cursor*2+parpadeo);
LCDRS=1;
}

void stepm1(char velocidad){
STEPM1=1;
__delay_us(DLY_LM317PULSE);
STEPM1=0;
if (velocidad==0) __delay_us(DLY_STEPM1);
if (velocidad==1) __delay_us(DLY_STEPM1_2);

STEPM1=1;
__delay_us(DLY_LM317PULSE);
STEPM1=0;
if (velocidad==0) __delay_us(DLY_STEPM1);
if (velocidad==1) __delay_us(DLY_STEPM1_2);
}

void stepm2(void){
STEPM2=1;
__delay_us(DLY_LM317PULSE);
STEPM2=0;
__delay_us(DLY_STEPM2);
}

void movemotor(char motor, unsigned int pasos){
	char vel=1;
	unsigned int pasosdados=0;

switch (motor) {
        case 0: for (;pasos!=0;pasos--) stepm1(0);
                break;
 
        case 1: if (pasos>100) {
					for (pasos=pasos*2;pasosdados!=pasos;) {
						
						STEPM1=1;
						__delay_us(DLY_LM317PULSE);
						STEPM1=0;
						switch (vel) {							//actuar segun la velocidad
							case 1: __delay_ms(5);
									break;
							case 2: __delay_ms(2);
									break;
							case 3: __delay_us(900);
									break;
						}
						
						pasosdados++;
						switch (pasosdados) {			//ir aumentando la velocidad
							case 10: vel=2;
								break;
							case 20: vel=3;
								break;
						}
					}
				} else {		// si no es conveniente incrementar la velocidad
					for (;pasos!=0;pasos--) stepm1(1);
				}
                break; 


        case 2: for (;pasos!=0;pasos--) stepm2();
                break;
}

}


void errorfincarro() {

apagarm1();
enviartexto(texto7);
while(1) {
        sound(1);
        __delay_ms(1000);
};

}



void sound(char tipo) {
unsigned short time;
unsigned char time2;


switch (tipo) {
        case 1: for (time2=0;time2<3;time2++) { //error
                                for (time=0;time<300;time++) {
                                        SOUND=1;
                                        __delay_us(100);
                                        SOUND=0;
                                        __delay_us(100);
                                };
                                __delay_ms(60);
                }               
                break;
        case 3:                                                 //aceptar-completado
                for (time=0;time<600;time++) {
                        SOUND=1;
                        __delay_us(80);
                        SOUND=0;
                        __delay_us(80);
                }
                for (time=0;time<1200;time++) {
                        SOUND=1;
                        __delay_us(60);
                        SOUND=0;
                        __delay_us(60);
                }
                for (time=0;time<1500;time++) {
                        SOUND=1;
                        __delay_us(40);
                        SOUND=0;
                        __delay_us(40);
                }               
                break;

}

}

void imprimirletra(unsigned char letraascii) {

unsigned char letrabraille=0;


//caracteres no imprimibles:

if (letraascii==0x20) { //espacio
        DIRM1=DIR_IZQ;
        movemotor(1,(ST_MEDIDA_B+ST_MEDIDA_C));
        letrasrenglon++;
        if ((letrasrenglon*(ST_MEDIDA_C+ST_MEDIDA_B))>ST_CARRO_POSINICIAL) {
                                DIRM2=DIR_IN;
                                movemotor(2,ST_MEDIDA_D);
                                DIRM1=DIR_DER;
                                movemotor(1,(letrasrenglon*(ST_MEDIDA_C+ST_MEDIDA_B)));
                                letrasrenglon=0;
        }

return;

};


if (letraascii==3) {    //enter
                                DIRM2=DIR_IN;
                                movemotor(2,ST_MEDIDA_D);
                                DIRM1=DIR_DER;
                                movemotor(1,(letrasrenglon*(ST_MEDIDA_C+ST_MEDIDA_B)));
                                letrasrenglon=0;
        return;
};

letrabraille = ascii2braille( letraascii , 0 );


if (((letrabraille)&(0b00000001))!=0) martillar();      //punto 1
DIRM2=DIR_IN;
movemotor(2,ST_MEDIDA_A);
if (((letrabraille)&(0b00000010))!=0) martillar();      //punto 2
movemotor(2,ST_MEDIDA_A);
if (((letrabraille)&(0b00000100))!=0) martillar(); //punto 3
DIRM1=DIR_IZQ;
movemotor(1,ST_MEDIDA_B);
if (((letrabraille)&(0b00100000))!=0) martillar(); //punto 6
DIRM2=DIR_OUT;
movemotor(2,ST_MEDIDA_A); //+ST_RODILLO_FIX);
if (((letrabraille)&(0b00010000))!=0) martillar(); //punto 5
movemotor(2,ST_MEDIDA_A);
if (((letrabraille)&(0b00001000))!=0) martillar();      //punto 4
movemotor(1,ST_MEDIDA_C);
DIRM2=DIR_IN;
movemotor(2,ST_RODILLO_FIX);

letrasrenglon++;
if ((letrasrenglon*(ST_MEDIDA_C+ST_MEDIDA_B))>ST_CARRO_POSINICIAL) {
                                DIRM2=DIR_IN;
                                movemotor(2,ST_MEDIDA_D);
                                DIRM1=DIR_DER;
                                encenderm1();
                                movemotor(1,(letrasrenglon*(ST_MEDIDA_C+ST_MEDIDA_B)));
                                letrasrenglon=0;
}


}


void martillar(void) {

MARTILLO=1;
__delay_ms(DLY_MARTILLO_ON);
MARTILLO=0;
__delay_ms(DLY_MARTILLO_OFF);


}

void saltodelinea(void){

        DIRM2=DIR_IN;
        movemotor(2,ST_MEDIDA_D);
        DIRM1=DIR_DER;
        movemotor(1,(letrasrenglon*(ST_MEDIDA_C+ST_MEDIDA_B)));
        letrasrenglon=0;

}

void pruebaeeprom(void) {


}

void initvisor(void) {


lcdclear();
lcdlinea(0);
lcdcursortype(0,1);
visoractivo=1;
visorcontletras=0;



}

void animacion(void) {

unsigned char i, j;



//escudo del krause moviendose
                lcdwritecg(4, EscudoA);
                lcdwritecg(5, EscudoB);
                lcdwritecg(6, EscudoC);
                lcdwritecg(7, EscudoD);
                j=0;
                while (1) {
                        lcdclear();
                        for (i=0;i<j;i++) {
                                lcdsend(' ');
                        };
                        lcdsend(4);
                        lcdsend(5);
                        lcdlinea(1);
                        for (i=0;i<j;i++) {
                                lcdsend(' ');
                        };
                        lcdsend(6);
                        lcdsend(7);
                        __delay_ms(500);
                        j++;
                        if (j==15) j=0;                 
                };



}


void encenderm1(void) {

ENABLEM1=1;
__delay_ms(DLY_M1_ON);

stepm1(0);
stepm1(0);
stepm1(0);
stepm1(0);


}

void apagarm1(void) {

ENABLEM1=0;


}

unsigned char ascii2braille( unsigned char letraascii , unsigned char letrabraille ) {

//si se quiere convertir ascii a braille , poner la letra en el primer parametro, y en el segundo 0.

//si se quiere convertir braille a ascii , poner la letra en el segundo parametro, y en el primero 0.

const unsigned char tabla[95]={0,51,62,0,55,46,12,44,59,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,38,60,0,0,0,0,35,28,20,0,2,36,4,0,0,0,0,0,0,0,0,0,0,0,0,6,0,0,0,34,0,1,3,9,25,17,11,27,19,10,26,5,7,13,29,21,15,31,23,14,30,37,39,58,45,61,53,0,0,0,40};
unsigned char ii=0;



if (letraascii==0 && letrabraille== 0) return 0;	//null
if (letraascii!=0 && letrabraille!= 0) return 0;	//error

if (letraascii!=0) {	//ascii a braille
	
	if (letraascii>94) return 0;

	return tabla[letraascii];


}

//braille a ascii


for (ii=0 ; ii<95 ; ii++) {	//buscar si el caracter esta en la tabla.

	if ( tabla[ii] == letrabraille ) return ii;

}

return (letrabraille+0b10000000);	//si no se conoce el caracter, guardarlo igual pero con el bit de "caracter braille"



}