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

double onset    = 0.0;
double relax    = 0.0;
double interval = 0.0;


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

void
buildPage (WiFiClient client)
{
#ifdef DO_SLIDER
#define initter(v)				\
  { \
    String holder = String ( #v " = ") + String (v, 2); \
    client.println (holder); \
    client.println ("if (searchParams.has('" #v "'))"); \
    client.println ("    " #v " = searchParams.get('" #v "');"); \
    client.println ("document.getElementById('" #v "').value = " #v ";");  \
    client.println ("document.getElementById('" #v "r').value = " #v ";"); \
  }
#else
#define initter(v)				\
  { \
    String holder = String ( #v " = ") + String (v, 2); \
    client.println (holder); \
    client.println ("if (searchParams.has('" #v "'))"); \
    client.println ("    " #v " = searchParams.get('" #v "');"); \
    client.println ("document.getElementById('" #v "').value = " #v ";");  \
  }
#endif
  
	    /***** scripts *******/

  client.println ("<script>");

// When the user clicks on div, open the popup
  client.println ("function showEditor() {");
  client.println ("  var popup = document.getElementById(\"myPopup\");");
  client.println ("  popup.classList.add(\"show\");");
  client.println ("  var text = document.getElementById('editor')");
  client.println ("  text.value = 'henry';");
  client.println ("}");

  client.println ("window.onload = function () {");
  client.println ("  let srch = window.location.search;");
  client.println ("  const searchParams = \
 new URLSearchParams(srch);");

  initter (pdx);
  initter (pdy);
  initter (pdz);
  initter (proll);
  initter (ppitch);
  initter (pyaw);
  initter (jdx);
  initter (jdy);
  initter (jdz);
  initter (jroll);
  initter (jpitch);
  initter (jyaw);
  initter (onset);
  initter (relax);
  initter (interval);
  client.println ("}"); // end window.onload function

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



	    
	    /**** function createURL(el) ****/
	    
  client.println ("function createURL(el) {");
  client.println ("  reloadApp (el);");
  client.println ("  return false;");
  client.println ("}");

	    
	    /**** function keyHandler(el) ****/

  client.println ("function keyHandler(el) {");
  client.println ("  if (event.keyCode=='13'){");
  client.println ("    reloadApp (el);");
  client.println ("    return false;");
  client.println ("  }");
  client.println ("  return true;");
  client.println ("}");
  
  client.println ("</script>");

	    /***** end scripts *******/


	    /*** begin page ****/
	    
  client.println ("<h1>Stewart</h2>");
	    
	    
	    /****  forms ****/
	    
#define BUILD_ETY(id,lbl,stp,fm)                              \
  client.println ("<td style=\"text-align:right\">"); \
  client.println (  "<label for=\"" #id "\">" #lbl "</label>"); \
  client.println ("</td>"); \
  client.println ("<td style=\"text-align:right\">"); \
  client.println (  "<input type=\"number\" id=\"" #id"\" step=\"" #stp "\" form=\"" #fm "\">"); \
  client.println ("</td>")
	    
#define BUILD_ETYM(id,lbl,stp,fm)                              \
  client.println ("<td style=\"text-align:right\">"); \
  client.println (  "<label for=\"" #id "\">" #lbl "</label>"); \
  client.println ("</td>"); \
  client.println ("<td style=\"text-align:right\">"); \
  client.println (  "<input type=\"number\" id=\"" #id"\" step=\"" #stp "\" form=\"" #fm "\" min=\"0\">"); \
  client.println ("</td>")

	    /**** position form ****/

  client.println ("<div>");			// start position form
  client.println ("<h2>Position</h2>");

  client.println ("<form id=\"position\" method=\"get\" \
            onchange=\"createURL(this);\"		       \
            onkeypress=\"return keyHandler(this);\"            \
            >");

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
	    
  client.println ("<form id=\"jitter\" method=\"get\" \
            onchange=\"createURL(this);\"		       \
            onkeypress=\"return keyHandler(this);\"            \
            >");
	    
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
	    
  client.print ("<form id=\"time\" method=\"get\" \
            onchange=\"createURL(this);\"			\
            onkeypress=\"return keyHandler(this);\"            \
            >");
	    
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
  
  client.println ("<script>");
  client.println ("function saveText() {");
  client.println ("  var text = window.location.origin + \"?text=\" + \
document.getElementById('editor').value;");
  client.println ("  window.location.href = text;");
  client.println ("  return false;");
  client.println ("}");
  
  client.println ("function abandonText() {");
  client.println ("  window.location.href = window.location.origin;");
  client.println ("  return false;");
  client.println ("}");
  client.println ("</script>");

  client.println ("<div class=\"popup\" \
onclick=\"showEditor()\">Open editor");

  client.println ("<span class=\"popuptext\" id=\"myPopup\">");
  client.println ("<textarea id=\"editor\" rows=\"5\" \
    method=\"get\"></textarea><br>");
  client.println ("<button type=\"button\" \
onclick=\"abandonText();\">Abandon</button>");
  client.println ("<button type=\"button\" \
onclick=\"saveText();\">Save</button>");
  client.println ("</span>");
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
        if (c == '\n') {            // if the byte is a newline character
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

	    client.println("<style>");

	    /* Popup container */
	    client.println (".popup {");
	    client.println ("position: relative;");
	    client.println ("display: inline-block;");
	    client.println ("cursor: pointer;");
	    client.println ("}");

/* The actual popup (appears on top) */
	    client.println (".popup .popuptext {");
	    client.println ("visibility: hidden;");
	    client.println ("width: 160px;");
	    client.println ("background-color: #555;");
	    client.println ("color: #fff;");
	    client.println ("text-align: center;");
	    client.println ("border-radius: 6px;");
	    client.println ("padding: 8px 0;");
	    client.println ("position: absolute;");
	    client.println ("z-index: 1;");
	    client.println ("bottom: 125%;");
	    client.println ("left: 50%;");
	    client.println ("margin-left: -80px;");
	    client.println ("}");


/* Popup arrow */
	    client.println (".popup .popuptext::after {");
	    client.println ("content: "";");
	    client.println ("position: absolute;");
	    client.println ("top: 100%;");
	    client.println ("left: 50%;");
	    client.println ("margin-left: -5px;");
	    client.println ("border-width: 5px;");
	    client.println ("border-style: solid;");
	    client.println ("border-color: #555 transparent \
transparent transparent;");
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

	    buildPage (client);
    
	    client.println ();
            break;
          }		// if empty line
	  else {	// non-empty line
#if 0
	    Serial.print ("\"");
	    Serial.print (currentLine);
	    Serial.println ("\"");
#endif
	    if (currentLine.startsWith("Referer:")) {
	      {
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
#ifdef DO_BLINK_TEST
		if (!isnan (jdx) &&
		    !isnan (jdy) &&
		    !isnan (jdz) &&
		    !isnan (jroll) &&
		    !isnan (jpitch) &&
		    !isnan (jyaw)
		    ) {
		  blink ((int)fabs (jdx));
		  blink ((int)fabs (jdy));
		  blink ((int)fabs (jdz));
		}
#endif
	      }		// if not editor
	    }			// if Referer
	    currentLine = "";
	  }			// non empty line
        }			// if '\n'
	else if (c != '\r') {
          currentLine += c;      // add it to the end of the currentLine
        }
      }				// if data available
    }				// if connected
    client.stop();
  }				// if client
}				// end of loop
