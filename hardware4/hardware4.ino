#include <SPI.h>
#include <WiFiNINA.h>

#define SECRET_SSID "NETGEAR80"
#define SECRET_PASS "magical574"

char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password 

int status = WL_IDLE_STATUS;

WiFiServer server(80);

double pdx    = 0.0;
double pdy    = 0.0;
double pdz    = 0.0;
double proll  = 0.0;
double ppitch = 0.0;
double pyaw   = 0.0;

double jdx    = 0.0;
double jdy    = 0.0;
double jdz    = 0.0;
double jroll  = 0.0;
double jpitch = 0.0;
double jyaw   = 0.0;

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

  server.begin();    // start the web server on port 80
  Serial.println("Connected...");

#ifdef DO_BLINK_TEST
  pinMode(LED_BUILTIN, OUTPUT);
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

#if 0
int
initFields (searchParams)
{
  String holder;

  holder = String ("pdx = ") + String (pdx, 2);
  client.println (holder);
  client.println ("if (searchParams.has('pdx'))");
  client.println ("    pdx = searchParams.get('pdx');");
  client.println ("document.getElementById('pdx').value = pdx;");
	    
  holder = String ("pdy = ") + String (pdy, 2);
  client.println (holder);
  client.println ("if (searchParams.has('pdy'))");
  client.println ("    pdy = searchParams.get('pdy');");
  client.println ("document.getElementById('pdy').value = pdy;");
	    
  holder = String ("pdz = ") + String (pdz, 2);
  client.println (holder);
  client.println ("if (searchParams.has('pdz'))");
  client.println ("    pdz = searchParams.get('pdz');");
  client.println ("document.getElementById('pdz').value = pdz;");
	    
  holder = String ("proll = ") + String (proll, 2);
  client.println (holder);
  client.println ("if (searchParams.has('proll'))");
  client.println ("    proll = searchParams.get('proll');");
  client.println ("document.getElementById('proll').value = proll;");
	    
  holder = String ("ppitch = ") + String (ppitch, 2);
  client.println (holder);
  client.println ("if (searchParams.has('ppitch'))");
  client.println ("    ppitch = searchParams.get('ppitch');");
  client.println ("document.getElementById('ppitch').value = ppitch;");
	    
  holder = String ("pyaw = ") + String (pyaw, 2);
  client.println (holder);
  client.println ("if (searchParams.has('pyaw'))");
  client.println ("    pyaw = searchParams.get('pyaw');");
  client.println ("document.getElementById('pyaw').value = pyaw;");

  holder = String ("jdx = ") + String (jdx, 2);
  client.println (holder);
  client.println ("if (searchParams.has('jdx'))");
  client.println ("    jdx = searchParams.get('jdx');");
  client.println ("document.getElementById('jdx').value = jdx;");
  
  holder = String ("jdy = ") + String (jdy, 2);
  client.println (holder);
  client.println ("if (searchParams.has('jdy'))");
  client.println ("    jdy = searchParams.get('jdy');");
  client.println ("document.getElementById('jdy').value = jdy;");
	    
  holder = String ("jdz = ") + String (jdz, 2);
  client.println (holder);
  client.println ("if (searchParams.has('jdz'))");
  client.println ("    jdz = searchParams.get('jdz');");
  client.println ("document.getElementById('jdz').value = jdz;");
	    
  holder = String ("jroll = ") + String (jroll, 2);
  client.println (holder);
  client.println ("if (searchParams.has('jroll'))");
  client.println ("    jroll = searchParams.get('jroll');");
  client.println ("document.getElementById('jroll').value = jroll;");
	    
  holder = String ("jpitch = ") + String (jpitch, 2);
  client.println (holder);
  client.println ("if (searchParams.has('jpitch'))");
  client.println ("    jpitch = searchParams.get('jpitch');");
  client.println ("document.getElementById('jpitch').value = jpitch;");
	    
  holder = String ("jyaw = ") + String (jyaw, 2);
  client.println (holder);
  client.println ("if (searchParams.has('jyaw'))");
  client.println ("    jyaw = searchParams.get('jyaw');");
  client.println ("document.getElementById('jyaw').value = jyaw;");
}
#endif
              
void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    // make a String to hold incoming data from the client
    String currentLine = "";
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
	
        char c = client.read();     // read a byte, then
        if (c == '\n') {            // if the byte is a newline character
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

	    client.println("<style>");
	    client.println("div {");
	    client.println("width: 720px;");
	    client.println("background-color: lightgrey;");
	    client.println("border: 15px lightgrey;");
	    client.println("padding: 15px;");
	    client.println("margin: 20px;");
	    client.println("}");
	    client.println("</style>");
	    
	    client.println ("<script>");
	    client.println ("window.onload = function () {");
	    client.println ("  let srch = window.location.search;");
	    client.println ("  const searchParams = new URLSearchParams(srch);");
#if 0
	    initFields (searchParams);
#else
{
  String holder;

  holder = String ("pdx = ") + String (pdx, 2);
  client.println (holder);
  client.println ("if (searchParams.has('pdx'))");
  client.println ("    pdx = searchParams.get('pdx');");
  client.println ("document.getElementById('pdx').value = pdx;");
	    
  holder = String ("pdy = ") + String (pdy, 2);
  client.println (holder);
  client.println ("if (searchParams.has('pdy'))");
  client.println ("    pdy = searchParams.get('pdy');");
  client.println ("document.getElementById('pdy').value = pdy;");
	    
  holder = String ("pdz = ") + String (pdz, 2);
  client.println (holder);
  client.println ("if (searchParams.has('pdz'))");
  client.println ("    pdz = searchParams.get('pdz');");
  client.println ("document.getElementById('pdz').value = pdz;");
	    
  holder = String ("proll = ") + String (proll, 2);
  client.println (holder);
  client.println ("if (searchParams.has('proll'))");
  client.println ("    proll = searchParams.get('proll');");
  client.println ("document.getElementById('proll').value = proll;");
	    
  holder = String ("ppitch = ") + String (ppitch, 2);
  client.println (holder);
  client.println ("if (searchParams.has('ppitch'))");
  client.println ("    ppitch = searchParams.get('ppitch');");
  client.println ("document.getElementById('ppitch').value = ppitch;");
	    
  holder = String ("pyaw = ") + String (pyaw, 2);
  client.println (holder);
  client.println ("if (searchParams.has('pyaw'))");
  client.println ("    pyaw = searchParams.get('pyaw');");
  client.println ("document.getElementById('pyaw').value = pyaw;");

  holder = String ("jdx = ") + String (jdx, 2);
  client.println (holder);
  client.println ("if (searchParams.has('jdx'))");
  client.println ("    jdx = searchParams.get('jdx');");
  client.println ("document.getElementById('jdx').value = jdx;");
  
  holder = String ("jdy = ") + String (jdy, 2);
  client.println (holder);
  client.println ("if (searchParams.has('jdy'))");
  client.println ("    jdy = searchParams.get('jdy');");
  client.println ("document.getElementById('jdy').value = jdy;");
	    
  holder = String ("jdz = ") + String (jdz, 2);
  client.println (holder);
  client.println ("if (searchParams.has('jdz'))");
  client.println ("    jdz = searchParams.get('jdz');");
  client.println ("document.getElementById('jdz').value = jdz;");
	    
  holder = String ("jroll = ") + String (jroll, 2);
  client.println (holder);
  client.println ("if (searchParams.has('jroll'))");
  client.println ("    jroll = searchParams.get('jroll');");
  client.println ("document.getElementById('jroll').value = jroll;");
	    
  holder = String ("jpitch = ") + String (jpitch, 2);
  client.println (holder);
  client.println ("if (searchParams.has('jpitch'))");
  client.println ("    jpitch = searchParams.get('jpitch');");
  client.println ("document.getElementById('jpitch').value = jpitch;");
	    
  holder = String ("jyaw = ") + String (jyaw, 2);
  client.println (holder);
  client.println ("if (searchParams.has('jyaw'))");
  client.println ("    jyaw = searchParams.get('jyaw');");
  client.println ("document.getElementById('jyaw').value = jyaw;");
}
#endif
	    client.println ("}");
	    client.println ("</script>");


	    client.println ("<script>");

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

	    client.println ("  default:");
	    client.println ( "   rc = window.location.origin \
    + '?jdx='    + el.jdx.value \
    + '&jdy='    + el.jdy.value \
    + '&jdz='    + el.jdz.value \
    + '&jroll='  + el.jroll.value \
    + '&jpitch=' + el.jpitch.value \
    + '&jyaw='   + el.jyaw.value;");
	    client.println ("    break;");
	    client.println ("  }");

	    client.println ("console.log ('rc = ');");
	    client.println ("console.log (rc);");
	    client.println ("  window.location.href = rc;");
	    client.println ("  return false;");
	    client.println ("}");



	    
	    /**** function createURL(el) ****/
	    
	    client.println ("function createURL(el) {");
	    client.println ("  reloadApp (el);");
	    client.println ("  return false;");
	    client.println ("}");

	    
	    /**** function keyHandler(el) ****/

	    client.println ("function keyHandler(el) {");
	    client.println ("console.log('kh = ');");
	    client.println ("console.log(event.keyCode);");
	    client.println ("  if (event.keyCode=='13'){");
	    Serial.println ("abt to rel");
	    client.println ("    reloadApp (el);");
	    client.println ("    return false;");
	    client.println ("  }");
	    client.println ("  return true;");
	    client.println ("}");

	    client.println ("</script>");


	    /*** begin page ****/
	    
	    client.println ("<h1>Stewart</h2>");
	    
	    
	    /****  forms ****/

	    /**** position form ****/

	    client.println ("<div>");
	    client.println ("<h2>Position</h2>");

	    client.print ("<form id=\"position\" method=\"get\" \
            onchange=\"createURL(this);\"			\
            onkeypress=\"return keyHandler(this);\"            \
            >");
	    
	    client.print ("<label for=\"pdx\">(cm) dx </label>");
	    client.print ("<input type=\"number\" id=\"pdx\" step=\"0.1\">  ");
	    
	    client.print ("<label for=\"pdy\">dy </label>");
	    client.print ("<input type=\"number\" id=\"pdy\" step=\"0.1\">  ");
    
	    client.print ("<label for=\"pdz\">dz </label>");
	    client.print ("<input type=\"number\" id=\"pdz\" step=\"0.1\"><br>");

	    client.print ("<br>");
    
	    client.print ("<label for=\"proll\">(deg) roll </label>");
	    client.print ("<input type=\"number\" id=\"proll\" step=\"1.0\">");
    
	    client.print ("<label for=\"ppitch\">pitch </label>");
	    client.print ("<input type=\"number\" id=\"ppitch\" step=\"1.0\">");
    
	    client.print ("<label for=\"pyaw\">yaw </label>");
	    client.print ("<input type=\"number\" id=\"pyaw\" step=\"1.0\"><br>");

	    client.print ("<br>");
	    
	    client.print ("<input type=\"submit\" value=\"Submit\"><br>");
	    
	    client.print("</form>");
	    client.println ("</div>");
	    


	    /**** jitter form ****/

	    client.println ("<div>");
	    client.println ("<h2>Jitter</h2>");
	    
	    client.print ("<form id=\"jitter\" method=\"get\" \
            onchange=\"createURL(this);\"			\
            onkeypress=\"return keyHandler(this);\"            \
            >");
	    
	    client.print ("<label for=\"jdx\">(cm) dx </label>");
	    client.print ("<input type=\"number\" id=\"jdx\" step=\"0.1\">  ");
	    
	    client.print ("<label for=\"jdy\">dy </label>");
	    client.print ("<input type=\"number\" id=\"jdy\" step=\"0.1\">  ");
    
	    client.print ("<label for=\"jdz\">dz </label>");
	    client.print ("<input type=\"number\" id=\"jdz\" step=\"0.1\"><br>");

	    client.print ("<br>");
    
	    client.print ("<label for=\"jroll\">(deg) roll </label>");
	    client.print ("<input type=\"number\" id=\"jroll\" step=\"1.0\">");
    
	    client.print ("<label for=\"jpitch\">pitch </label>");
	    client.print ("<input type=\"number\" id=\"jpitch\" step=\"1.0\">");
    
	    client.print ("<label for=\"jyaw\">yaw </label>");
	    client.print ("<input type=\"number\" id=\"jyaw\" step=\"1.0\"><br>");

	    client.print ("<br>");
	    
	    client.print ("<input type=\"submit\" value=\"Submit\"><br>");
	    
	    client.print("</form>");
	    client.println ("</div>");
    
            client.println();
            break;
          } else {
            if (currentLine.startsWith("Referer:")) {
              int startPos = 0;
              parseString(jdx,    currentLine, "jdx=",    startPos);
              parseString(jdy,    currentLine, "jdy=",    startPos);
              parseString(jdz,    currentLine, "jdz=",    startPos);
              parseString(jroll,  currentLine, "jroll=",  startPos);
              parseString(jpitch, currentLine, "jpitch=", startPos);
              parseString(jyaw,   currentLine, "jyaw=",   startPos);
	      startPos = 0;
              parseString(pdx,    currentLine, "pdx=",    startPos);
              parseString(pdy,    currentLine, "pdy=",    startPos);
              parseString(pdz,    currentLine, "pdz=",    startPos);
              parseString(proll,  currentLine, "proll=",  startPos);
              parseString(ppitch, currentLine, "ppitch=", startPos);
              parseString(pyaw,   currentLine, "pyaw=",   startPos);
	      if (!isnan (jdx) &&
		  !isnan (jdy) &&
		  !isnan (jdz) &&
		  !isnan (jroll) &&
		  !isnan (jpitch) &&
		  !isnan (jyaw)
		  ) {
#ifdef DO_BLINK_TEST
		blink ((int)fabs (jdx));
		blink ((int)fabs (jdy));
		blink ((int)fabs (jdz));
#endif
	      }
            }
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    client.stop();
  }
}
