#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdexcept>
#include <cmath> // Für sin() und cos()
#include <vector>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

class myvector{
  public:
    float x, y, z;

    // Default constructor
    myvector() : x(0), y(0), z(0) {}

    // Parameterized constructor
    myvector(float x, float y, float z) : x(x), y(y), z(z) {}

    myvector(std::initializer_list<float> values) {
        if (values.size() != 3) {
            throw std::invalid_argument("Initializer list must contain 3 values.");
        }
        auto it = values.begin();
        x = *(it++);
        y = *(it++);
        z = *it;
    }

    std::vector<std::vector<float>> get_Matrix() const {
        return {
            {x},
            {y},
            {z}
        };
    }

};

#define R1 3 // number of rows in Matrix-1
#define C1 3 // number of columns in Matrix-1
#define R2 3 // number of rows in Matrix-2
#define C2 1 // number of columns in Matrix-2

myvector matmul(float mat1[][C1], myvector vec)
{
    float rslt[R1][C2];

    for (int i = 0; i < R1; i++) {
        for (int j = 0; j < C2; j++) {
            rslt[i][j] = 0;
            for (int k = 0; k < R2; k++) {
                rslt[i][j] += mat1[i][k] * vec.get_Matrix()[k][j];
            }
        }
    }

    return myvector(rslt[0][0], rslt[1][0], rslt[2][0]);
}

float rotationX[R1][C1];
void updateRotation_X(float angle_deg, float rotationX[][C1])
{
    float angle_rad = angle_deg * (M_PI / 180.0);
    float sin_val = sin(angle_rad);
    float cos_val = cos(angle_rad);

    rotationX[0][0] = 1;
    rotationX[0][1] = 0;
    rotationX[0][2] = 0;
    rotationX[1][0] = 0;
    rotationX[1][1] = cos_val;
    rotationX[1][2] = -sin_val;
    rotationX[2][0] = 0;
    rotationX[2][1] = sin_val;
    rotationX[2][2] = cos_val;
}

float rotationY[R1][C1];
void updateRotation_Y(float angle_deg, float rotationY[][C1])
{
    float angle_rad = angle_deg * (M_PI / 180.0);
    float sin_val = sin(angle_rad);
    float cos_val = cos(angle_rad);

    rotationY[0][0] = cos_val;
    rotationY[0][1] = 0;
    rotationY[0][2] = sin_val;
    rotationY[1][0] = 0;
    rotationY[1][1] = 1;
    rotationY[1][2] = 0;
    rotationY[2][0] = -sin_val;
    rotationY[2][1] = 0;
    rotationY[2][2] = cos_val;
}

float rotationZ[R1][C1];
void updateRotation_Z(float angle_deg, float rotationZ[][C1])
{
    float angle_rad = angle_deg * (M_PI / 180.0);
    float sin_val = sin(angle_rad);
    float cos_val = cos(angle_rad);

    rotationZ[0][0] = cos_val;
    rotationZ[0][1] = -sin_val;
    rotationZ[0][2] = 0;
    rotationZ[1][0] = sin_val;
    rotationZ[1][1] = cos_val;
    rotationZ[1][2] = 0;
    rotationZ[2][0] = 0;
    rotationZ[2][1] = 0;
    rotationZ[2][2] = 1;
}

float projection[2][3] = {
  {1, 0, 0},
  {0, 1, 0}
};

void connect(int i, int j, myvector *Points) {
    int x1 = round(Points[i].x);
    int y1 = round(Points[i].y);
    int x2 = round(Points[j].x);
    int y2 = round(Points[j].y);
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
    // Serial.print("[(");
    // Serial.print(x1);
    // Serial.print("|");
    // Serial.print(y1);
    // Serial.print(")(");
    // Serial.print(x2);
    // Serial.print("|");
    // Serial.print(y2);
    // Serial.println("]");
}

const float offset_x = SCREEN_WIDTH / 2;
const float offset_y = SCREEN_HEIGHT / 2;
float angle = 0.0;
float l = 10;
myvector Cube[] = {
  myvector(l,-l,-l),
  myvector(l,l,-l),
  myvector(-l,l,-l),
  myvector(-l,-l,-l),
  myvector(l,-l,l),
  myvector(l,l,l),
  myvector(-l,l,l),
  myvector(-l,-l,l)
};

void setup() {  
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();

  display.setTextSize(1);      
  display.setTextColor(SSD1306_WHITE); 
  display.cp437(true);   
}

void loop()
{
  display.clearDisplay();
  myvector data_setup[8];
  for(int i=0;i<8;i++){
    updateRotation_X(angle, rotationX);
    updateRotation_Y(angle, rotationY);
    updateRotation_Z(angle, rotationZ);
    myvector rotated = matmul(rotationX, Cube[i]);
    rotated = matmul(rotationY, rotated);
    rotated = matmul(rotationZ, rotated);
    myvector projected = matmul(projection, rotated);
    projected.x += offset_x;
    projected.y += offset_y;
    //display.drawPixel(projected.x, projected.y, SSD1306_WHITE);
    data_setup[i] = projected;

    // Serial.print("[");
    // Serial.print(projected.x);
    // Serial.print("|");
    // Serial.print(projected.y);
    // Serial.println("]");
  }
  for(int i=0;i<4;i++){
    connect(i, (i+1)%4, data_setup);
    connect(i+4, ((i+1)%4)+4, data_setup);
    connect(i, (i+4), data_setup);
  }
  display.display();

  angle += 2.0;
  if(angle > 360.0){
    angle = 0.0;
  }
}