#include <ESP8266WiFi.h>
#include <Wire.h>

#define ADXL_ADDR           0x53        // Device address as specified in data sheet
#define REG_POWER_CTL       0x2D        // Power Control Register
#define REG_DATA_FORMAT     0x31        // Data Format Register
#define REG_DATA_X0         0x32        // x, y, and z are 2 bytes each starting at this addr


void writeADXL(byte address, byte val)
{
  Wire.beginTransmission(ADXL_ADDR);
  Wire.write(address);
  Wire.write(val);
  Wire.endTransmission();
}

void readADXL(byte address, int num, byte _buff[])
{
  int i = 0;
  Wire.beginTransmission(ADXL_ADDR);
  Wire.write(address);
  Wire.endTransmission();
  Wire.beginTransmission(ADXL_ADDR);
  Wire.requestFrom(ADXL_ADDR, num);
  while (Wire.available())
    _buff[i++] = Wire.read();
  Wire.endTransmission();
}


void initAccel()
{
  Wire.begin(0,2);

  // Put the ADXL345 into +/- 2G range by writing the value
  // 0x01 to the DATA_FORMAT register.

  writeADXL(REG_DATA_FORMAT, 0x00);

  // Put the ADXL345 into Measurement Mode by writing
  // 0x08 to the POWER_CTL register.

  writeADXL(REG_POWER_CTL, 0x08);

}


void _readAccel(short *x, short *y, short *z)
{
    byte buf[6];
    readADXL(REG_DATA_X0,6,buf);

    *x = (((short)buf[1]) << 8) | buf[0];
    *y = (((short)buf[3]) << 8) | buf[2];
    *z = (((short)buf[5]) << 8) | buf[4];
}


void readAccel(short *fx, short *fy, short *fz)
{
    #define NUM_SAMPLES     20

    #define xDataOffset     2
    #define yDataOffset     -14
    #define zDataOffset     -290

    #define scaleFactor     15

    static int tx[NUM_SAMPLES];
    static int ty[NUM_SAMPLES];
    static int tz[NUM_SAMPLES];

    static int j = 0;
    short x,y,z;
    _readAccel(&x,&y,&z);

    if (0)
    {
        Serial.print(x);
        Serial.print(",");
        Serial.print(y);
        Serial.print(",");
        Serial.print(z);
        Serial.print("    ");
    }

    tx[j] = scaleFactor * (x + xDataOffset);
    ty[j] = scaleFactor * (y + yDataOffset);
    tz[j] = scaleFactor * (z + zDataOffset);
    j++;

    if (j == NUM_SAMPLES)
        j = 0;

    int ix = 0;
    int iy = 0;
    int iz = 0;

    for (int i=0; i<NUM_SAMPLES; i++)
    {
        ix += tx[i];
        iy += ty[i];
        iz += tz[i];
    }

    ix /= NUM_SAMPLES;
    iy /= NUM_SAMPLES;
    iz /= NUM_SAMPLES;

    #define xDisplayOffset 10
    #define yDisplayOffset 100
    #define zDisplayOffset 200

    if (0)
    {
        Serial.print(ix);  // +xDisplayOffset);
        Serial.print(",");
        Serial.print(iy);  // +yDisplayOffset);
        Serial.print(",");
        Serial.print(iz);  // +zDisplayOffset);
        Serial.println();
    }

    *fx = ix;
    *fy = iy;
    *fz = iz;
}
