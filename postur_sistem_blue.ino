#include <SoftwareSerial.h>  // bluetooth kütüphanesi
#include<Math.h>   //Trigonometrik işlemler için kütüphane 
#include <Wire.h>  // ı2c kütüphanesi

SoftwareSerial kambur(7, 6); // RX,TX
const int MPU_addr = 0x68;                        //MPU6050 I2C Slave adresi
unsigned long oncekiMillis = 0;                   //Önceki millis değeri için değişken
const long interval = 30;                         //Ölçümler arasındaki süre (ms) yani sistemin step rate değeri 
//(millis unsigned long veri tipinde sayı döndürür. Bu fonksiyon her milisaniyede artan bir sayaç olarak çalışıyor. Değerler milisaniye cinsinden tutulduğu için
//geri döndürülen değerler int float ın tutabileceğinin çok ustunde olabilir bu yüzden unsigned )

float ang_x;  //x y ve z için değişkenler
float ang_y;  
float ang_z;  
 
float X_Error =1.35;   // x ve y için hata değerleri
float Y_Error =1.64;
 
void setup() {
  kambur.begin(9600);                                    // Bluetooth için haberleşme başlatır
  Serial.begin(9600);                                    //Seri haberleşme başlatılır 9600 saniye başına gönderilen ve alınan bit
  
  Wire.begin();                                          //I2C Başlatılır (Değer girilmedi böylece haberleşme master olarak başlatıldı) 
  Wire.beginTransmission(MPU_addr);                      // Master aygıttan slave aygıta veri gönderimini başlatılır buradaki adres slave aygıtın adresidir
  Wire.write(0x6B);                                      // 6B regesterını resetledik
  Wire.write(0x00);                                      // mpu çalıştırıldı
  Wire.endTransmission(true);                            //beginTransmission ile başlattığım veri iletimini bitir
}                                                        //I2C haberleşmesi başlatıldı ve MPU-6050'nin ilk ayarları yapıldı
 
void loop() {
  unsigned long simdikiMillis = millis();               //Millis fonksiyonu çağrılır ve millis değeri alınır
 
  if (simdikiMillis - oncekiMillis >= interval) {      //Şimdiki zaman ile son eğim değerimi aldığım zaman arasındaki fark eğim değerini almak istediğim zamandan büyükse (aralık süresinin geçip geçmediği kontrol edilir)
    oncekiMillis = simdikiMillis;                      //Eğimi aldığım son zamanı kaydeder. Önceki mill geçerli mile eşittir
 
    Wire.beginTransmission(MPU_addr);                    //MPU6050 ile I2C haberleşme başlatılır
    Wire.write(0x3B);                                    //(ACCEL_XOUT_H) accel için okuma başlat 0*3b adresindeki registere ulaşıldı
    Wire.endTransmission(false);                         // beginTransmissinon ile başlatılan iletimi sonlandırır false ile bağlantıyı aktif tutar ve yeniden başlatma gönderir
    Wire.requestFrom(MPU_addr, 6, true);                 // Veri istenir ilk parametre veriyi isteyeceğimiz slave adresi ikini parametre kaç bytlık veri olduğu true veriyolunu serbest bırakır
   
    int16_t XEksen =  (Wire.read() << 8 | Wire.read()); //Sırası ile okunan her iki byte birleştirilerek sırayla değişkenlere yazdırılır (0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L))
    int16_t YEksen =  (Wire.read() << 8 | Wire.read()); //bu şekilde mpu dan tüm değrler okunur (0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L))
    int16_t ZEksen =  (Wire.read() << 8 | Wire.read()); //Tüm değerlerin okunabilmesi için 0*3b adresinden başlanıldı  (0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L))
    // int16_t 16 bit 2 byte  yani bu değişkenin boyutu sabittir
    
    ang_x = (atan(YEksen/(sqrt(pow(XEksen,2)+pow(ZEksen,2)))) * 180 / PI)+X_Error; //Euler Açı formülüne göre açı hesabı.(X-Ekseni)[ atan arktanjantı alır,sqrt karekök alır,pow üstünü hesaplar]
    ang_y = (atan(-1*XEksen/(sqrt(pow(XEksen,2)+pow(ZEksen,2)))) * 180 / PI)+Y_Error; //Euler Açı formülüne göre açı hesabı. (Y-Ekseni)[burada 180 / pi 57,296 sonucunu verecektir]
 
   
    Serial.print("X angle = "); //X ve Y eksenleri için açı değerleri seri porttan basılır
    Serial.print(ang_x);
    Serial.print("");
    Serial.print("\t");
    Serial.print("Y angle = ");
    Serial.println(ang_y);
    int x_acisi = ang_x;
    kambur.print(ang_x);            // X açısı değrleri bluetooth ile uygulamaya gönderilir
    kambur.print(";");

   if(x_acisi <70){                // Motorun titreşimi için koşul
     digitalWrite(13, HIGH);
     delay(300);
    digitalWrite(13,LOW);
    }else{
       digitalWrite(13, LOW);
      }
    delay(100);
 
  }
}
