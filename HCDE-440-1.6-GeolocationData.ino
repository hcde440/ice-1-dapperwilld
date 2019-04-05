
#include <ESP8266WiFi.h>      //includes library for ESP8266 wifi connectivity
#include <ESP8266HTTPClient.h>// includes library for ESP8266 web browsing capabilities
#include <ArduinoJson.h>      // library for JSON parser

const char* ssid = "yeetbois";    // variable containing ssid for wifi connection
const char* pass = "petersch";    // variable containing password for wifi connection
const char* key = "57a76f98e5489a608f02895ef4843559"; //API key for geolocation API
String weatherKey = "2fec1143f43e5bc01d052646a2cb8e1c"; //API key for open weather API

typedef struct {    // data structure (name: value pairs) that holds values for geolocation data
  String ip;
  String cc;
  String cn;
  String rc;
  String rn;
  String cy;
  String tz;
  String ln;
  String lt;
} GeoData;

typedef struct { // data structure (name: value pairs) that holds values for Metropolitan weather data
  String tp;
  String pr;
  String hd;
  String ws;
  String wd;
  String cd;
} MetData;

GeoData location;// creates Geodata structure named location 
MetData conditions; // creates Metdata structure named conditions

void setup() {
  Serial.begin(115200);
  delay(10);

  // Serial output communicating the file uploded to the arduino and the date it was compiled
  Serial.print("This board is running: ");
  Serial.println(F(__FILE__));
  Serial.print("Compiled: ");
  Serial.println(F(__DATE__ " " __TIME__));
  Serial.print("Connecting to "); Serial.println(ssid);

  // Begins wifi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  // Checks against wifi status to tell when the Arduino is connected to the internet
  // prints dots in Serial while Arduino is NOT connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println(); Serial.println("WiFi connected"); Serial.println();// Prints notification of connection to Serial
  Serial.print("Your ESP has been assigned the internal IP address "); 
  Serial.println(WiFi.localIP()); // Prints local IP address

  String ipAddress = getIP(); // Calls function getIP(); that retrieves external IP address
  getGeo(ipAddress); // Calls getGeo(); function passes in external IP address

  Serial.println();

  // Prints the location data from the location Geodata structure
  Serial.println("Your external IP address is " + location.ip);
  Serial.print("Your ESP is currently in " + location.cn + " (" + location.cc + "),");
  Serial.println(" in or near " + location.cy + ", " + location.rc + ".");
  Serial.print("You are in the " + location.tz + " timezone ");
  Serial.println("and located at (roughly) ");
  Serial.println(location.lt + " latitude by " + location.ln + " longitude.");
  
  getMet(location.cy); // Calls getMet function using the city variable from the location struct

  Serial.println();
  // Prints weather data using the conditions MetData structure
  Serial.println("With " + conditions.cd + ", the temperature in " + location.cy + ", " + location.rc);
  Serial.println("is " + conditions.tp + "F, with a humidity of " + conditions.hd + "%. The winds are blowing");
  Serial.println(conditions.wd + " at " + conditions.ws + " miles per hour, and the ");
  Serial.println("barometric pressure is at " + conditions.pr + " millibars.");
}

void loop() {
}
// function that retrieves IP address
String getIP() {
  HTTPClient theClient; // initializes browser
  String ipAddress; // declares ip address variable
  
  theClient.begin("http://api.ipify.org/?format=json"); //navigates browser to ip address API
  int httpCode = theClient.GET(); //sends GET request to page

  if (httpCode > 0) {
    if (httpCode == 200) { // two if statements check whether a successful GET has occured

      DynamicJsonBuffer jsonBuffer;// initializes JSON parser 

      String payload = theClient.getString(); //converts page contents to a String
      JsonObject& root = jsonBuffer.parse(payload); //JSON buffer PArses String as JSON. Stores it in root variable
      ipAddress = root["ip"].as<String>(); // sets IP address from the JSON ip value

    }
    else { // error message in case of error code from page
      Serial.println("Something went wrong with connecting to the endpoint in getIP()."); 
      return "error";
    }
  }
  return ipAddress; // returns the IP address
}
// function that retrieves Geodata information
void getGeo(String ipAddress) {
  HTTPClient theClient; // initializes browser
  Serial.println("Making HTTP request"); //prints message to Serial indicating request was made
  theClient.begin("http://api.ipstack.com/" + ipAddress + "?access_key=" + key); //navigates to page with geolocation JSON  
  int httpCode = theClient.GET(); //sends GET request to page

  if (httpCode > 0) {
    if (httpCode == 200) { // two if statements check whether a successful GET has occured
      Serial.println("Received HTTP payload."); // message output to Serial indicating payload recieved
      DynamicJsonBuffer jsonBuffer; // initializes JSON parser 
      String payload = theClient.getString(); // Converts page contents to string and stores it as payload variable
      Serial.println("Parsing...");//message output to Serial indicating parsing in progress
      JsonObject& root = jsonBuffer.parse(payload); //JSON buffer Parses String as JSON. Stores it in root variable 

      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(payload);
        return;
      }

      //Some debugging lines below:
      // Serial.println(payload);
      //      root.printTo(Serial);

      //Using .dot syntax, we refer to the variable "location" which is of
      //type GeoData, and place our data into the data structure.

      location.ip = root["ip"].as<String>();            //we cast the values as Strings b/c
      location.cc = root["country_code"].as<String>();  //the 'slots' in GeoData are Strings
      location.cn = root["country_name"].as<String>(); 
      location.rc = root["region_code"].as<String>();
      location.rn = root["region_name"].as<String>();           ///Populates the location GeoData
      location.cy = root["city"].as<String>();                  ///structure with the corresponding 
      location.lt = root["latitude"].as<String>();              ///JSON value 
      location.ln = root["longitude"].as<String>();

    } else {
      Serial.println("Something went wrong with connecting to the endpoint."); // catches error and prints a message in Serial
    }
  }
}

void getMet(String city) {
  HTTPClient theClient; // initializes browser
  String apiCall = "http://api.openweathermap.org/data/2.5/weather?q=" + city; //Assembles the URL for the 
  apiCall += "&units=imperial&appid=";                                         //Openweathermap API request and stores
  apiCall += weatherKey;                                                       //it in the apiCall variable
  theClient.begin(apiCall);//navigates to weather data API
  int httpCode = theClient.GET();//
  if (httpCode > 0) {

    if (httpCode == HTTP_CODE_OK) {//Checks whether the GET request was successful
      String payload = theClient.getString();
      DynamicJsonBuffer jsonBuffer;//initializs JSON parser
      JsonObject& root = jsonBuffer.parseObject(payload);//Parses String payload as a JSON object stored in root variable
      if (!root.success()) {//Checks whether parse was successful, prints error message to Serial
        Serial.println("parseObject() failed in getMet().");
        return;
      }
      conditions.tp = root["main"]["temp"].as<String>();
      conditions.pr = root["main"]["pressure"].as<String>();                 ///Populates the conditions MetData
      conditions.hd = root["main"]["humidity"].as<String>();                 ///structure with the corresponding
      conditions.cd = root["weather"][0]["description"].as<String>();        ///JSON values 
      conditions.ws = root["wind"]["speed"].as<String>();
      int deg = root["wind"]["deg"].as<int>();
      conditions.wd = getNSEW(deg);
    }
  }
  else {
    Serial.printf("Something went wrong with connecting to the endpoint in getMet().");//Checks whether parse was successful, prints error message to Serial
  }
}
// Function converts wind degrees as stored in the conditions metData structure to a String
// ie 35 = northeast 
String getNSEW(int d) {
  String direct;

  //Conversion based upon http://climate.umn.edu/snow_fence/Components/winddirectionanddegreeswithouttable3.htm
  if (d > 348.75 && d < 360 || d >= 0  && d < 11.25) {
    direct = "north";
  };
  if (d > 11.25 && d < 33.75) {
    direct = "north northeast";
  };
  if (d > 33.75 && d < 56.25) {
    direct = "northeast";
  };
  if (d > 56.25 && d < 78.75) {
    direct = "east northeast";
  };
  if (d < 78.75 && d < 101.25) {
    direct = "east";
  };
  if (d < 101.25 && d < 123.75) {
    direct = "east southeast";
  };
  if (d < 123.75 && d < 146.25) {
    direct = "southeast";
  };
  if (d < 146.25 && d < 168.75) {
    direct = "south southeast";
  };
  if (d < 168.75 && d < 191.25) {
    direct = "south";
  };
  if (d < 191.25 && d < 213.75) {
    direct = "south southwest";
  };
  if (d < 213.25 && d < 236.25) {
    direct = "southwest";
  };
  if (d < 236.25 && d < 258.75) {
    direct = "west southwest";
  };
  if (d < 258.75 && d < 281.25) {
    direct = "west";
  };
  if (d < 281.25 && d < 303.75) {
    direct = "west northwest";
  };
  if (d < 303.75 && d < 326.25) {
    direct = "south southeast";
  };
  if (d < 326.25 && d < 348.75) {
    direct = "north northwest";
  };
  return direct;
}
