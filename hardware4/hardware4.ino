#include <SPI.h>
#include <SD.h>
#include <WiFiNINA.h>
#include <JOAAT.h>

JOAAT joaat;

#define SECRET_SSID "NETGEAR80"
#define SECRET_PASS "magical574"

char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password 

int status = WL_IDLE_STATUS;

IPAddress ip;

WiFiServer server(80);

typedef struct {
  double val;
  uint32_t hash;
  const char *name;
} parm_s;

enum {
  PARM_PDX,
  PARM_PDY,
  PARM_PDZ,
  PARM_PROLL,
  PARM_PPITCH,
  PARM_PYAW,
  PARM_JDX,
  PARM_JDY,
  PARM_JDZ,
  PARM_JROLL,
  PARM_JPITCH,
  PARM_JYAW,
  PARM_ONSET,
  PARM_RELAX,
  PARM_INTERVAL
};

parm_s parms[] = {
  {0.0, 0, "pdx"},
  {0.0, 0, "pdy"},
  {0.0, 0, "pdz"},
  {0.0, 0, "proll"},
  {0.0, 0, "ppitch"},
  {0.0, 0, "pyaw"},
  {0.0, 0, "jdx"},
  {0.0, 0, "jdy"},
  {0.0, 0, "jdz"},
  {0.0, 0, "jroll"},
  {0.0, 0, "jpitch"},
  {0.0, 0, "jyaw"},
  {0.0, 0, "onset"},
  {0.0, 0, "relax"},
  {0.0, 0, "interval"}
};
  
#if 0
uint32_t pdxHash;
uint32_t pdyHash;
uint32_t pdzHash;
uint32_t prollHash;
uint32_t ppitchHash;
uint32_t pyawHash;

uint32_t jdxHash;
uint32_t jdyHash;
uint32_t jdzHash;
uint32_t jrollHash;
uint32_t jpitchHash;
uint32_t jyawHash0;

uint32_t onsetHash;
uint32_t orelaxHash;
uint32_t intervalHash;
#endif



void setup() {
  
  // check for the WiFi module:

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:

  Serial.print("Connecting to ");
  Serial.println(ssid);                   // print the network name (SSID);
  delay(1000);   // wait 1 second
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.println("Waiting...");
    delay(1000);   // wait 10 seconds for connection:
  }

  ip = WiFi.localIP();

  server.begin();    // start the web server on port 80
  
  Serial.println("Connected...");


#ifdef DO_BLINK_TEST
  pinMode(LED_BUILTIN, OUTPUT);
#endif

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }

#if 0
  File myFile = SD.open("test.txt", FILE_WRITE);
  
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");
     
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
    SD.remove("test.txt");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  Sd2Card card;
  SdVolume volume;
  SdFile root;

  const int chipSelect = 4;

  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("no go");
  }
  else {
    Serial.print("Card type:         ");
    switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
    }
	
    if (!volume.init(card)) {
      Serial.println("Could not find FAT16/FAT32 partition.\n\
Make sure you've formatted the card");
      while (1);
    }
	
    Serial.print("Clusters:          ");
    Serial.println(volume.clusterCount());
    
    Serial.print("Blocks x Cluster:  ");
    Serial.println(volume.blocksPerCluster());
	
    Serial.print("Total Blocks:      ");
    Serial.println(volume.blocksPerCluster() * volume.clusterCount());
	
    Serial.println();
	
    uint32_t volumesize;

    Serial.print("Volume type is:    FAT");
    Serial.println(volume.fatType(), DEC);
    volumesize = volume.blocksPerCluster();  
    volumesize *= volume.clusterCount();    
    volumesize /= 2; // SD card blocks are always 512 bytes (2 blocks are 1KB)

    Serial.print("Volume size (Kb):  ");
    Serial.println(volumesize);
	
    Serial.print("Volume size (Mb):  ");
    volumesize /= 1024;
    Serial.println(volumesize);
	
    Serial.print("Volume size (Gb):  ");
    Serial.println((float)volumesize / 1024.0);
	
    Serial.println("\nFiles found on the card \
(name, date and size in bytes): ");
	
    root.openRoot(volume);
	
    // list all files in the card with date and size
    
    root.ls(LS_R | LS_DATE | LS_SIZE);
    
    root.close();
  }
#endif

  parms[PARM_PDX].hash 		= joaat.encode_str (JOAAT_STR ("pdx"));
  parms[PARM_PDY].hash 		= joaat.encode_str (JOAAT_STR ("pdy"));
  parms[PARM_PDZ].hash		= joaat.encode_str (JOAAT_STR ("pdz"));
  parms[PARM_PROLL].hash	= joaat.encode_str (JOAAT_STR ("proll"));
  parms[PARM_PPITCH].hash	= joaat.encode_str (JOAAT_STR ("ppitch"));
  parms[PARM_PYAW].hash		= joaat.encode_str (JOAAT_STR ("pyaw"));

  parms[PARM_JDX].hash		= joaat.encode_str (JOAAT_STR ("jdx"));
  parms[PARM_JDY].hash		= joaat.encode_str (JOAAT_STR ("jdy"));
  parms[PARM_JDZ].hash		= joaat.encode_str (JOAAT_STR ("jdz"));
  parms[PARM_JROLL].hash	= joaat.encode_str (JOAAT_STR ("jroll"));
  parms[PARM_JPITCH].hash	= joaat.encode_str (JOAAT_STR ("jpitch"));
  parms[PARM_JYAW].hash		= joaat.encode_str (JOAAT_STR ("jyaw"));

  parms[PARM_ONSET].hash	= joaat.encode_str (JOAAT_STR ("onset"));
  parms[PARM_RELAX].hash	= joaat.encode_str (JOAAT_STR ("relax"));
  parms[PARM_INTERVAL].hash	= joaat.encode_str (JOAAT_STR ("interval"));
  
#if 0
  Serial.println("Calculated JOAAT Hash: ");
  auto jooat_HASH = joaat.encode_str(str);
  Serial.println(jooat_HASH, HEX);
  Serial.println("Is JOAAT string valid: ");
  Serial.println(joaat.validate_string_checksum(str, 0x5F3CC755) ? "true" : "false");
#endif
}

#ifdef DO_BLINK_TEST
static void
blink (int ct)
{
  for (int i = 0; i < ct; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);	
    delay(1000); 
  }
  delay(3000); 
}
#endif


#if 0
int
bsearch
#endif

#if 0
void
parseString(double &val, String currentLine, String tgt, int &startPos)
{
  double rc = NAN;
  int foundPos;
  if (-1 != (foundPos = currentLine.indexOf(tgt, startPos))) {
    int endPos = foundPos + tgt.length ();
    for (;endPos < currentLine.length(); endPos++) 
      if (!isDigit(currentLine.charAt(endPos)) &&
	  currentLine.charAt(endPos) != '-' &&
	  currentLine.charAt(endPos) != '.' ) break;
    String vv = currentLine.substring(foundPos+tgt.length (), endPos);
    rc = atof(vv.c_str());
    if (!isnan (rc)) val = rc;
#if 0
    Serial.print(tgt); Serial.println(val);
#endif
  }
}
#endif

void
initter (WiFiClient client, int idx)
{
  client.println ("{");
  String holder = String (parms[idx].name) + " = "
    + String (parms[idx].val, 2) + ";";
  client.println (holder);
  
  holder = String ("if (searchParams.has('") + parms[idx].name  + "'))";
  client.println (holder);

  holder = String (parms[idx].name) + " = searchParams.get('" +
    parms[idx].name + "');";
  client.println (holder);

  holder = String ("document.getElementById('")
    +  parms[idx].name + "').value = "
    + String(parms[idx].val) + ";";
  client.println (holder);
  client.println ("}");
}

void
buildPage (WiFiClient client)
{
#if 0
#ifdef DO_SLIDER
#define initter(v)				\
  { \
    String holder = String ( v " = ") + String (v, 2); \
    client.println (holder); \
    client.println ("if (searchParams.has(" + v "'))"); \
    client.println ("    " v " = searchParams.get('" v "');"); \
    client.println ("document.getElementById('" v "').value = " v ";");  \
    client.println ("document.getElementById('" v "r').value = " v ";"); \
  }
#else
#define initter(name,val)				\
  { \
    String holder = name + " = " + String (val, 2); \
    client.println (holder); \
    client.println ("if (searchParams.has(" + name + "))"); \
    client.println ("    " + name + " = searchParams.get(" + name + ");"); \
    client.println ("document.getElementById(" + name + ").value = " + String(val) + ";"); \
  }
#endif
#endif
  
	    /***** scripts *******/

  client.println ("<script>");

  /***
      {
        "name": "whatever",
	"position": [ "dx": "val:, "dy": "val", "dz": "val",
	   "roll": "val", "pitch": "val", "yaw": "val"],
	"jitter": [ "dx": "val:, "dy": "val", "dz": "val"],
	   "roll": "val", "pitch": "val", "yaw": "val"],
	"time": [ "onset": "val:, "relax": "val", "interval": "val"]
      }
   ***/

  // text.value = '{' + '  "name": "", + '}';
// When the user clicks on div, open the popup
  client.println ("function showEditor() {");
  client.println ("  var popup = document.getElementById(\"myPopup\");");
  client.println ("  popup.classList.add(\"show\");");
  client.println ("  var text = document.getElementById('editor')");
  client.println ("  text.value = \
'{\\n' + \
'  \"name\": \"\",\\n' + \
'  \"position\": [ ' + \
'\"dx\": \"'    + position.pdx.value + '\", ' + \
'\"dy\": \"'    + position.pdy.value + '\", ' + \
'\"dz\": \"'    + position.pdz.value + '\",\\n ' + \
'    \"roll\": \"'  + position.proll.value + '\", ' + \
'\"pitch\": \"' + position.ppitch.value + '\", ' + \
'\"yaw\": \"'   + position.pyaw.value + '\"],\\n ' + \
'  \"jitter\": [ ' + \
'\"dx\": \"'    + jitter.jdx.value + '\", ' + \
'\"dy\": \"'    + jitter.jdy.value + '\", ' + \
'\"dz\": \"'    + jitter.jdz.value + '\",\\n ' + \
'    \"roll\": \"'  + jitter.jroll.value + '\", ' + \
'\"pitch\": \"' + jitter.jpitch.value + '\", ' + \
'\"yaw\": \"'   + jitter.jyaw.value + '\"],\\n ' + \
'  \"time\": [ ' + \
'\"onset\": \"'    + time.onset.value + '\", ' + \
'\"relax\": \"'    + time.relax.value + '\", ' + \
'\"interval\": \"' + time.interval.value + '\"]\\n ' + \
'}';");
  client.println ("}");

  client.println ("window.onload = function () {");
  client.println ("  let srch = window.location.search;");
  client.println ("  const searchParams = \
 new URLSearchParams(srch);");

  initter (client, PARM_PDX);
  initter (client, PARM_PDY);
  initter (client, PARM_PDZ);
  initter (client, PARM_PROLL);
  initter (client, PARM_PPITCH);
  initter (client, PARM_PYAW);
  initter (client, PARM_JDX);
  initter (client, PARM_JDY);
  initter (client, PARM_JDZ);
  initter (client, PARM_JROLL);
  initter (client, PARM_JPITCH);
  initter (client, PARM_JYAW);
  initter (client, PARM_ONSET);
  initter (client, PARM_RELAX);
  initter (client, PARM_INTERVAL);

  client.println ("}"); // end window.onload function

#if 1
	    /**** function reloadApp(el) ****/
	    
  client.println ("function reloadApp(el) {");
  client.println (" let rc;");
  client.println ("  switch (el.id) {");
  client.println ("  case 'position':");
  client.println ( "   rc = window.location.origin \
    + '?pdx='    + el.pdx.value \
    + '&pdy='    + el.pdy.value \
    + '&pdz='    + el.pdz.value \
    + '&proll='  + el.proll.value \
    + '&ppitch=' + el.ppitch.value \
    + '&pyaw='   + el.pyaw.value;");
  client.println ("    break;");

  client.println ("  case 'jitter':");
  client.println ( "   rc = window.location.origin \
    + '?jdx='    + el.jdx.value \
    + '&jdy='    + el.jdy.value \
    + '&jdz='    + el.jdz.value \
    + '&jroll='  + el.jroll.value \
    + '&jpitch=' + el.jpitch.value \
    + '&jyaw='   + el.jyaw.value;");
  client.println ("    break;");

  client.println ("  default:");	// time
  client.println ( "   rc = window.location.origin \
    + '?onset='    + el.onset.value \
    + '&relax='    + el.relax.value \
    + '&interval=' + el.interval.value;");
  client.println ("    break;");

  client.println ("  }");		// end switch

  client.println ("  window.location.href = rc;");
  client.println ("  return false;");
  client.println ("}");
#endif



	    

  client.println ("function updateParam(el) {");

  client.println ("console.log('in uP');");
  client.println ("const XHR = new XMLHttpRequest();");

#if 0
  client.println ("XHR.onload = function() {");
  client.println ("  alert(`Loaded: ${XHR.status} ${XHR.response}`);");
  client.println ("};");

  client.println ("XHR.onerror = function() {");
  client.println ("  alert(`Network Error`);");
  client.println ("};");

  client.println ("XHR.onprogress = function(event) {");
  client.println ("  alert(`Received ${event.loaded} of ${event.total}`);");
  client.println ("};");
#endif

  client.println ("  var text = window.location.origin + \"?update=\" + \
el.id + \"=\" + el.value;");
  client.println ("XHR.open('POST', text);");
  client.println ("XHR.setRequestHeader('Content-Type', 'text/plain');");
  client.println ("XHR.send();");

  client.println ("}");
	    
	    /**** function keyHandler(el) ****/

  client.println ("function keyHandler(el) {");
  client.println ("  if (event.keyCode=='13'){");
  client.println ("    updateParam (el);");
  client.println ("    return false;");
  client.println ("  }");
  client.println ("  return true;");
  client.println ("}");
	    
  
  client.println ("</script>");

	    /***** end scripts *******/

  /********** styles ***********/

  client.println("<style>");

  client.println (".editor{");
  client.println ("width: 560px;");
  client.println ("height: 180px;");
  client.println ("rows: 5;");
  client.println ("cols: 50;");
  client.println ("wrap: off;");
  client.println ("background-color: lightgrey;");
  client.println ("}");
	    
	    /* Popup container */
  client.println (".popup {");
  client.println ("position: relative;");
  client.println ("display: inline-block;");
  client.println ("cursor: pointer;");
  client.println ("}");

/* The actual popup (appears on top) */
  client.println (".popup .popuptext {");
  client.println ("visibility: hidden;");
  client.println ("width: 560px;");
  client.println ("height: 180px;");
  client.println ("background-color: #555;");
  client.println ("color: #fff;");
  client.println ("text-align: center;");
  client.println ("border-radius: 6px;");
  client.println ("padding: 8px 0;");
  client.println ("position: absolute;");
  client.println ("z-index: 1;");
  client.println ("bottom: 125%;");
  client.println ("left: 30%;");
  client.println ("margin-left: -80px;");
  client.println ("}");


/* Popup arrow */
  client.println (".popup .popuptext::after {");
  client.println ("content: "";");
  client.println ("position: absolute;");
  client.println ("top: 100%;");
  client.println ("left: 30%;");
  client.println ("margin-left: -5px;");
  client.println ("border-width: 5px;");
  client.println ("border-style: solid;");
  client.println ("border-color: #555 transparent transparent transparent;");
  client.println ("}");

/* Toggle this class when clicking on the
   popup container (hide and show the popup) */
  client.println (".popup .show {");
  client.println ("visibility: visible;");
  client.println ("}");


  client.println("th, td {");
  client.println("  padding-top: 0px;");
  client.println("  padding-bottom: 0px;");
  client.println("  padding-left: 10px;");
  client.println("  padding-right: 0px;");
  client.println("}");
  
  client.println("div {");
  client.println("width: 820px;");
  client.println("background-color: lightgrey;");
  client.println("border: 15px lightgrey;");
  client.println("padding: 15px;");
  client.println("margin: 20px;");
  client.println("}");
  
  client.println("</style>");

	    /*** begin page ****/
	    
  client.println ("<h1>Stewart</h2>");
	    
	    
	    /****  forms ****/

#define BUILD_ETY(id,lbl,stp,fm)		      \
  client.println ("<td style=\"text-align:right\">"); \
  client.println (  "<label for=\"" #id "\">" #lbl "</label>"); \
  client.println ("</td>"); \
  client.println ("<td style=\"text-align:right\">"); \
  client.println (  "<input type=\"number\" id=\"" #id"\" step=\"" #stp "\" onchange=\"updateParam(this);\">"); \
  client.println ("</td>")
	    
#define BUILD_ETYM(id,lbl,stp,fm)                              \
  client.println ("<td style=\"text-align:right\">"); \
  client.println (  "<label for=\"" #id "\">" #lbl "</label>"); \
  client.println ("</td>"); \
  client.println ("<td style=\"text-align:right\">"); \
  client.println (  "<input type=\"number\" id=\"" #id"\" step=\"" #stp "\" onchange=\"updateParam(this);\" onkeypress=\"return keyHandler(this);\" min=\"0\">"); \
  client.println ("</td>")
  
	    /**** position form ****/

  client.println ("<div>");			// start position form
  client.println ("<h2>Position</h2>");

  client.println ("<form id=\"position\" method=\"get\">");

  client.println ("<table>");

#ifdef DO_SLIDER
  client.println ("  <tr>");
	    
  client.println ("  <td>");	// empty units
  client.println ("  </td>");
	    
  client.println ("  <td>");	// empty id
  client.println ("  </td>");
	    
  client.println ("  <td>");
  client.println ("  <input type=\"range\" min=\"-4\" max=\"4\" \
value=\"50\" class=\"slider\" id=\"pdxr\" form=\"position\" value=\"0\">");
  client.println ("  </td>");
  client.println ("  </tr>");

#endif
  client.println ("  <tr>");		// start pos offset row

  client.println ("    <td style=\"text-align:right\">");
  client.println ("(cm)");
  client.println ("    </td>");

  BUILD_ETY (pdx, dx, 0.1, position);
  BUILD_ETY (pdy, dy, 0.1, position);
  BUILD_ETY (pdz, dz, 0.1, position);

  client.println ("  </tr>");		// end pos offset row
  
  client.println ("  <tr>");		// start pos attitude row

  client.println ("    <td style=\"text-align:right\">");
  client.println ("(deg)");
  client.println ("    </td>");
	    
  BUILD_ETY (proll,  roll,  1.0, position);
  BUILD_ETY (ppitch, pitch, 1.0, position);
  BUILD_ETY (pyaw,   yaw,   1.0, position);
  
  client.println ("  </tr>");		// end pos attitude row

  client.println ("</table>");

  client.print("</form>");

  client.println ("</div>");		// end position form
	    


	    /**** jitter form ****/

  client.println ("<div>");		// start jitter form
  client.println ("<h2>Jitter</h2>");
	    
  client.println ("<form id=\"jitter\" method=\"get\">");
	    
  client.println ("<table>");
  
  client.println ("  <tr>");

  client.println ("    <td style=\"text-align:right\">");
  client.println ("(cm)");
  client.println ("    </td>");

  BUILD_ETY (jdx, dx, 0.1, jitter);
  BUILD_ETY (jdy, dy, 0.1, jitter);
  BUILD_ETY (jdz, dz, 0.1, jitter);
  
  client.println ("  </tr>");

  client.println ("  <tr>");

  client.println ("    <td style=\"text-align:right\">");
  client.println ("(deg)");
  client.println ("    </td>");

  BUILD_ETY (jroll,  roll,  1.0, jitter);
  BUILD_ETY (jpitch, pitch, 1.0, jitter);
  BUILD_ETY (jyaw,   yaw,   1.0, jitter);
  
  client.println ("  </tr>");
  
  client.println ("</table>");
	 
  client.println ("</form>");
  
  client.println ("</div>");		// end jitter form
  


	    /**** time form ****/

  client.println ("<div>");
  client.println ("<h2>Time</h2>");
	    
  client.println ("<form id=\"time\" method=\"get\">");
	    
  client.println ("<table>");

  client.println ("  <tr>");

  client.println ("    <td style=\"text-align:right\">");
  client.println ("(secs)");
  client.println ("    </td>");

  BUILD_ETYM (onset,    onset,    0.1, time);
  BUILD_ETYM (relax,    relax,    0.1, time);
  BUILD_ETYM (interval, interval, 0.1, time);

  client.println ("  </tr>");

  client.println ("</table>");

  client.println("</form>");

  client.println ("</div>");		// end time form


	    /**** editor popup ****/

  // http://arduino/?text={%22name%22%20:%20%22george%22;%22stuff%22%20:%20{%20%20%20%20%22thing%22%20:%20%22gadget;}}"

  // %22 = doublequote
  // $20 = space

  // https://stackoverflow.com/questions/2367979/pass-post-data-with-window-location-href



client.println ("<script>");
#if 1
  client.println ("function saveText() {");

  client.println ("const XHR = new XMLHttpRequest();");
  

   client.println ("  var text = window.location.origin + \"?text=\" + \
document.getElementById('editor').value;");
  client.println ("XHR.open('POST', text);");
  client.println ("XHR.setRequestHeader('Content-Type', 'text/plain');");
  client.println ("XHR.send();");

  client.println ("}");
 
  
#else
  client.println ("function saveText() {");
  client.println ("  var text = window.location.origin + \"?text=\" + \
document.getElementById('editor').value;");
  client.println ("  window.location.href = text;");
  client.println ("  return false;");
  client.println ("}");
#endif
  
  client.println ("function abandonText() {");
  client.println ("  window.location.href = window.location.origin;");
  client.println ("  return false;");
  client.println ("}");
  client.println ("</script>");


  // https://www.w3schools.com/js/js_ajax_http_send.asp
  
   client.println ("<div class=\"popup\" \
onclick=\"showEditor()\">Open editor");

#if 1
  client.println ("<span class=\"popuptext\" id=\"myPopup\" \
style=\"width:560px\">");
#else
  client.println ("<form class=\"popuptext\" id=\"myPopup\">");
#endif

  client.println ("<textarea id=\"editor\" rows=\"9\" cols=\"20\" \
    method=\"post\" name=\"george\" wrap=\"off\" \
style=\"width:512px;minWidth=512px;\
height=360px;minHeight=180px;\
background-color=lightgrey\"></textarea><br>");

  client.println ("<button type=\"button\" \
onclick=\"abandonText();\">Abandon</button>");

  client.println ("<button type=\"button\" \
onclick=\"saveText();\">Save</button>");

#if 0
  client.println ("</form>");
#else
  //  client.println ("</span>");
#endif

  client.println ("</div>");


  

  
  
#if 0
  /********* upload form *******/
  
  client.println ("<div>");
  client.println ("<form id=\"uploadbanner\" \
enctype=\"multipart/form-data\" method=\"post\" action=\"#\">");
  client.println ("  <input id=\"fileupload\" \
name=\"myfile\" type=\"file\" \
accept=\"text/plain,application/json.application/xml\"/>");
  client.println ("  <input type=\"submit\" value=\"submit\" \
id=\"submit\" />");
  client.println ("</form>");
  client.println ("</div>");
#endif


	    /***** end of forms ********/
}


              
void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    // make a String to hold incoming data from the client
    String currentLine = "";
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
	
        char c = client.read();     // read a byte, then
	Serial.print (c);
        if (c == '\n') {            // if the byte is a newline character
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
	    
	    buildPage (client);

	    client.println ();
            break;
          }		// if empty line
	  else {	// non-empty line
	    //	    Serial.println (currentLine);
	    if (currentLine.startsWith("POST")) {
	      String startString = "update=";
	      int startPos = currentLine.indexOf(startString);
	      if (-1 != startPos) {
		startPos += startString.length ();
		String endString = "HTTP";
		int endPos = currentLine.indexOf(endString);
		if (-1 != endPos) {
		  endPos--;
		  String text = currentLine.substring (startPos, endPos);
		  int endPos =  text.indexOf ("=");
		  String vbl = text.substring (0, endPos);
		  String vals = text.substring (1 + endPos);
		  double val = vals.toDouble ();

		  }
	      }
	    }
	  }
	  currentLine = "";
        }			// if '\n'
	else if (c != '\r') {
          currentLine += c;      // add it to the end of the currentLine
        }
      }				// if data available
    }				// if connected
    client.stop();
  }				// if client
}				// end of loop
