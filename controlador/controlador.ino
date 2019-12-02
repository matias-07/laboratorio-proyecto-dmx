// ************************ Controlador DMX con Arduino ************************
//
// El programa lee la entrada de cuatro botones pulsadores y envía la señal
// a una luz compatible con el protocolo DMX.
// Tres de estos botones son utilizados para seleccionar un canal a controlar,
// (R, G o B) y el otro para seleccionar un efecto de luces predefinido.
// En el caso de los canales, se utiliza el potenciómetro para controlar
// la intensidad de las luces, mientras que si es seleccionado el efecto, se lo
// utiliza para controlar la velocidad del mismo.
// 
// *****************************************************************************

#include <DMXSerial.h>
#include <Bounce2.h>

#define MODO_MASTER     2   // Modo master (controlador)
#define BOTON_EFECTO    6
#define BOTON_RED       5
#define BOTON_GREEN     4
#define BOTON_BLUE      3
#define PIN_NIVEL       A0  // Potenciómetro

// La dirección base es la dirección de la luz DMX, los offsets se le suman
// para obtener la dirección de la función específica a controlar
byte direccion_base = 5;
byte offset_efecto = 6;
byte offset_red = 1;
byte offset_green = 2;
byte offset_blue = 3;

// Variables para guardar el offset y el nivel del potenciómetro actual
byte offset;
byte nivel;
byte offsets[3] = {1, 2, 3};

// Botones
const int debounce_interval = 10;
Bounce boton_red_bounce = Bounce();
Bounce boton_green_bounce = Bounce();
Bounce boton_blue_bounce = Bounce();
Bounce boton_efecto_bounce = Bounce();

void setup() {
    DMXSerial.init(DMXController, MODO_MASTER);

    pinMode(BOTON_RED, INPUT_PULLUP);
    pinMode(BOTON_BLUE, INPUT_PULLUP);
    pinMode(BOTON_GREEN, INPUT_PULLUP);
    pinMode(BOTON_EFECTO, INPUT_PULLUP);

    boton_red_bounce.attach(BOTON_RED);
    boton_red_bounce.interval(debounce_interval);
    boton_green_bounce.attach(BOTON_GREEN);
    boton_green_bounce.interval(debounce_interval);
    boton_blue_bounce.attach(BOTON_BLUE);
    boton_blue_bounce.interval(debounce_interval);
    boton_efecto_bounce.attach(BOTON_EFECTO);
    boton_efecto_bounce.interval(debounce_interval);
    DMXSerial.write(direccion_base + 0, 200);  
}

void efecto(int velocidad) {
    for (int i = 0; i < 3; i++) {
        DMXSerial.write(direccion_base + offsets[i], 200);
        delay(velocidad);
        DMXSerial.write(direccion_base + offsets[i], 0);
        delay(velocidad);
    }
}

void pruevaDeMedicionOndaCuadrada_1() {
    // el 170 en decimal es el 1010 1010 en binario deberia dar una onda cuadrada repite la onda 1010101010101010
    DMXSerial.write(170, 170);
}
void pruevaDeMedicionOndaCuadrada_2() {
    // repite la onda 0000000011111111
    DMXSerial.write(0, 255);
}
void pruevaDeMedicionOndaCuadrada_3() {
    // repite la onda 00000000000000001
    DMXSerial.write(0, 1);
}

void loop() {
    // Obtenemos el valor del potenciómetro (entre 1 y 255)
    nivel = map(analogRead(PIN_NIVEL), 0, 1023, 1, 255);

    // Obtenemos el offset actualmente seleccionado (R, G, B o efecto)
    boton_red_bounce.update();
    boton_green_bounce.update();
    boton_blue_bounce.update();
    boton_efecto_bounce.update();

    if (boton_red_bounce.fell()) {
        offset = offset_red;
    }
    if (boton_green_bounce.fell()) {
        offset = offset_green;
    }
    if (boton_blue_bounce.fell()) {
        offset = offset_blue;
    }
    if (boton_efecto_bounce.fell()) {
        offset = offset_efecto;
    }

    if (offset == offset_efecto) {
      // A niveles altos del potenciómetro, se pasa al control manual.
      // De esta forma no hay problemas de sincronía para salir del efecto.
      if (nivel > 240) {
        offset = offset_red;
      } else {
        // Acá el nivel "pasa a ser" la velocidad del efecto.
        efecto(int(nivel));
      }
    } else {
      DMXSerial.write(direccion_base + offset, nivel);
    }
}
