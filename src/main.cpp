#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <EEPROM.h>

// #define verboseOutput 1
// #define denganLcd 1
#define dirPinStepper 4
#define stepPinStepper 5
#define enablePinStepper 6

AccelStepper myStepper(AccelStepper::DRIVER, stepPinStepper, dirPinStepper);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int nilai_input[4] = {0, 0, 0, 0};
int nilai_input_sebelumnya[4] = {0, 0, 0, 0};
int nilai_debounce_input_sebelumnya[4] = {0, 0, 0, 0};
int nilai_delay_lcd_sebelumnya = 0;

const int nilai_debounce = 20;
const int nilai_delay_lcd = 500;

// variabel rotasi
int kecepatan_rotasi_welding = 0;
bool arah_rotasi_welding = false;
bool start_stop_rotasi_welding = true;

// pin output
#define output_1 13
#define output_2 12
#define output_3 11
#define output_4 10
int output_pins[4] = {output_1, output_2, output_3, output_4};

// pin input
#define input_1 A0
#define input_2 A1
#define input_3 A2
#define input_4 A3
int input_pins[4] = {input_1, input_2, input_3, input_4};

void setup()
{
	EEPROM.get(10, kecepatan_rotasi_welding); // kecepatan rotasi
											  // EEPROM.get(20, arah_rotasi_welding);      // arah rotasi

#ifdef verboseOutput
	Serial.begin(115200);
	Serial.println("START");
#endif

#ifdef denganLcd
	lcd.init(); // initialize the lcd
	lcd.backlight();
	lcd.setCursor(0, 0);
	lcd.print("rotasi welding!");
#endif

	// setting up some default values for maximum speed and maximum acceleration
	myStepper.setMaxSpeed(100000);	   // SPEED = Steps / second
	myStepper.setAcceleration(100000); // ACCELERATION = Steps /(second)^2
	myStepper.disableOutputs();		   // disable outputs, so the motor is not getting warm (no current)

	for (size_t i = 0; i < 4; i++) // ada 8 buah output
		pinMode(output_pins[i], OUTPUT);
	for (size_t i = 0; i < 4; i++) // ada 10 buah output
		pinMode(input_pins[i], INPUT_PULLUP);
}

void loop()
{
	myStepper.run();

	for (int i = 0; i < 4; i++)
	{
		// Read the state of the button
		int reading = digitalRead(input_pins[i]);

		// Check if the button state has changed
		if (reading != nilai_input_sebelumnya[i])
		{
			nilai_debounce_input_sebelumnya[i] = millis();
		}

		// If the button state has remained the same for longer than the debounce delay
		if ((millis() - nilai_debounce_input_sebelumnya[i]) > nilai_debounce)
		{
			// Update the button state
			if (reading != nilai_input[i])
			{
				nilai_input[i] = reading;
				// If the button is pressed, toggle the LED state
				// if (nilai_input[i] == HIGH)
				// {
				//     digitalWrite(output_pins[i], !digitalRead(output_pins[i]));
				// }
				if (nilai_input[0] == LOW) // karena trigger low
				{
					EEPROM.put(10, kecepatan_rotasi_welding); // feedrate
					kecepatan_rotasi_welding = kecepatan_rotasi_welding + 30;
				}
				else if (nilai_input[1] == LOW)
				{
					EEPROM.put(10, kecepatan_rotasi_welding); // feedrate
					kecepatan_rotasi_welding = kecepatan_rotasi_welding - 30;
					kecepatan_rotasi_welding < 0 ? kecepatan_rotasi_welding = 0 : kecepatan_rotasi_welding;
				}
				else if (nilai_input[2] == LOW)
				{
					arah_rotasi_welding = !arah_rotasi_welding;
					// EEPROM.put(20, arah_rotasi_welding);      // akselerasi
				}
				else if (nilai_input[3] == LOW)
				{
					start_stop_rotasi_welding = !start_stop_rotasi_welding;
				}
			}
		}

		// Save the reading for the next loop iteration
		nilai_input_sebelumnya[i] = reading;
	}

	// start dan stop putaran
	digitalWrite(output_pins[0], start_stop_rotasi_welding);
	// tampilkan arah putaran
	digitalWrite(output_pins[1], arah_rotasi_welding);
	digitalWrite(dirPinStepper, arah_rotasi_welding);
	// tampilkan nilai kecepatan
	analogWrite(output_pins[2], kecepatan_rotasi_welding);

	// jalankan motor
	if (start_stop_rotasi_welding) // hanya dieksekusi sekali, pada boot
	{
		myStepper.setSpeed(kecepatan_rotasi_welding);
		// myStepper.runSpeed();
		arah_rotasi_welding ? myStepper.moveTo(99999) : myStepper.moveTo(-99999);
#if defined(verboseOutput)
		Serial.print("motor run");
#endif // verboseOutput
	}
	else
	{
		myStepper.stop();
		myStepper.setCurrentPosition(0);
#if defined(verboseOutput)
		Serial.print("motor stop");
#endif // verboseOutput
	}

// Check if the button state has changed
#ifdef denganLcd
	nilai_delay_lcd_sebelumnya = millis();
	// If the button state has remained the same for longer than the debounce delay
	if ((millis() - nilai_delay_lcd_sebelumnya) > nilai_debounce)
	{
		// Update the button state
		lcd.init(); // initialize the lcd
		lcd.backlight();
		lcd.setCursor(0, 0);
		lcd.print("kec: ");
		lcd.print(kecepatan_rotasi_welding);
		lcd.setCursor(0, 1);
		lcd.print("arah: ");
		lcd.print(arah_rotasi_welding);
	}
#endif

// tampilkan nilai
#if defined(verboseOutput)
	Serial.print("inp: ");
	for (size_t i = 0; i < 4; i++)
		Serial.print(nilai_input[i]);
	Serial.print(" kec: ");
	Serial.print(kecepatan_rotasi_welding);
	Serial.print(" arah: ");
	Serial.print(arah_rotasi_welding);
	Serial.print(" start: ");
	Serial.print(start_stop_rotasi_welding);
	Serial.println();
#endif // verbose
}
