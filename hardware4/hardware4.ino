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

	    client.println("<?php");
	    client.println("  $uploaddir = \"uploads/\";");
	    client.println("  $uploadfile = $uploaddir . \
basename( $_FILES['file']['name']);");

	    client.println("  if(move_uploaded_file($_FILES['file']['tmp_name'], \
$uploadfile)) {");
	    client.println("    echo \"The file has been uploaded successfully\";");
	    client.println("  } else {");
	    client.println("    echo \"There was an error uploading the file\";");
	    client.println("  }");
	    client.println("?>");
	    
	    client.println ("<script>");
	    client.println ("window.onload = function () {");
	    client.println ("  let srch = window.location.search;");
	    client.println ("  const searchParams = \
 new URLSearchParams(srch);");

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

	    client.println ("}");
	    client.println ("</script>");


	    client.println ("<script>");

#if 0
	    //https://www.geeksforgeeks.org/how-to-make-ajax-call-from-javascript/
	    client.println ("function ajaxCall() {");
            client.println ("  $.ajax({");
  
            client.println ("    url: 'https://jsonplaceholder.typicode.com/todos/1',");
            client.println ("    type: \"GET\",");
  
            client.println ("    success: function (data) {");
            client.println ("      var x = JSON.stringify(data);");
            client.println ("      console.log(x);");
	    client.println ("    },");
  
            client.println ("    error: function (error) {");
	    client.println ("      console.log(`Error ${error}`);");
            client.println ("    }");
            client.println ("  });");
	    client.println ("}");
	    client.println ("ajaxCall();");
#endif

#if 0
	    client.println ("function displayFullName() {");
	    client.println ("  var request = new XMLHttpRequest();");

	    client.println ("  var url = '/home/moller/mydata.txt';");
	    client.println ("  request.open(\"GET\", url, true);");

	    client.println ("  request.onreadystatechange = function() {");
	    client.println ("  if(this.readyState === 4 && this.status === 200) {");
            client.println ("    document.getElementById(\"result\").innerHTML \
= this.responseText;");
	    client.println ("  console.log(\"resp\");");
	    client.println ("  console.log(this.responseText);");
	    client.println ("  }");
	    client.println ("};");

	    client.println ("request.send();");
	    client.println ("}");
#endif


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

	    client.println ("<div>");
	    client.println ("<h2>Position</h2>");

	    client.print ("<form id=\"position\" method=\"get\" \
            onchange=\"createURL(this);\"			\
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

	    client.println ("  <tr>");
#endif

	    client.println ("    <td style=\"text-align:right\">");
            client.print ("(cm)");
            client.println ("    </td>");

	    BUILD_ETY (pdx, dx, 0.1, position);
	    BUILD_ETY (pdy, dy, 0.1, position);
	    BUILD_ETY (pdz, dz, 0.1, position);

	    client.println ("  </tr>");

	    client.println ("  <tr>");

	    client.println ("    <td style=\"text-align:right\">");
            client.print ("(deg)");
            client.println ("    </td>");
	    
	    BUILD_ETY (proll,  roll,  1.0, position);
	    BUILD_ETY (ppitch, pitch, 1.0, position);
	    BUILD_ETY (pyaw,   yaw,   1.0, position);
	    
	    client.println ("  </tr>");

	    client.println ("  <tr>");

	    client.println ("  </tr>");

	    client.println ("</table>");

	    client.print("</form>");

	    client.println ("</div>");		// end position form
	    


	    /**** jitter form ****/

	    client.println ("<div>");
	    client.println ("<h2>Jitter</h2>");
	    
	    client.print ("<form id=\"jitter\" method=\"get\" \
            onchange=\"createURL(this);\"			\
            onkeypress=\"return keyHandler(this);\"            \
            >");
	    
	    client.println ("<table>");

	    client.println ("  <tr>");

	    client.println ("    <td style=\"text-align:right\">");
            client.print ("(cm)");
            client.println ("    </td>");

	    BUILD_ETY (jdx, dx, 0.1, jitter);
	    BUILD_ETY (jdy, dy, 0.1, jitter);
	    BUILD_ETY (jdz, dz, 0.1, jitter);

	    client.println ("  </tr>");

	    client.println ("  <tr>");

	    client.println ("    <td style=\"text-align:right\">");
            client.print ("(deg)");
            client.println ("    </td>");

	    BUILD_ETY (jroll,  roll,  1.0, jitter);
	    BUILD_ETY (jpitch, pitch, 1.0, jitter);
	    BUILD_ETY (jyaw,   yaw,   1.0, jitter);
	    
	    client.println ("  </tr>");

	    client.println ("  <tr>");

	    client.println ("  </tr>");

	    client.println ("</table>");
	 
	    client.print("</form>");

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
            client.print ("(secs)");
            client.println ("    </td>");

	    BUILD_ETYM (onset,    onset,    0.1, time);
	    BUILD_ETYM (relax,    relax,    0.1, time);
	    BUILD_ETYM (interval, interval, 0.1, time);

	    client.println ("  </tr>");

	    client.println ("  </tr>");
	    
	    client.println ("</table>");

	    client.print("</form>");

	    client.println ("</div>");		// end time form


	    /**** editor form ****/

	    client.println ("<form action=\"/form/submit\" method=\"GET\">");
	    client.println ("  <textarea rows=\"5\" cols=\"60\" \
name=\"text\" placeholder=\"Enter text\"></textarea>");
	    client.println ("  <br/>");
	    client.println ("  <input type=\"submit\" value=\"submit\"/>");
	    client.println ("</form>");

#if 0
	    /****** ajax button ********/
	    client.println ("<div id=\"result\">");
	    client.println ("  <p>Content of the result DIV box will be \
replaced by the server response</p>");
	    client.println ("</div>");
	    client.println ("<button type=\"button\" \
onclick=\"displayFullName()\">Display Full Name</button>");
#endif

	    //#define DO_UPLOAD_FORM
#ifdef DO_UPLOAD_FORM
	    /**** upload form ****/

#if 0
	    client.println ("<form method=\”post\” \
enctype=\”multipart/form-data\”>");
	    client.println ("  <div>");
	    client.println ("    <label for=\”script_uploads\”>\
 Choose any script to upload (txt, json, xml)</label>");
	    client.println ("    <input type=\”file\” id=\”script_uploads\” \
name=\”script_uploads\” accept=”.txt, .json, .xml” multiple>");
	    client.println ("  </div>");
	    client.println ("  <div class=\”preview\”>");
	    client.println ("    <p> No files are currently selected for \
upload </p>");
	    client.println ("  </div>");
	    client.println ("  <div>");
	    client.println ("    <button> Click here to submit</button>");
	    client.println ("  </div>");
	    client.println ("</form>");
#endif
	    
#if 0
	    client.println ("<form action=\"uoload.php\" \
enctype=\"multipart/form-data\" method=\"post\">");
	    client.println ("  <label class=\"custom\" \
for=\"file\">Upload Your File</label>");
	    client.println ("  <input id=\"file\" \
accept=\"text/plain,application/json.application/xml\" \
name=\"fileToUpload\" type=\"file\" />");
	    client.println ("  <button class=\"btn btn-success\" \
name=\"submit\" type=\"submit\"> Upload File </button>");
	    client.println ("</form>");
#endif
	    
#if 0
	    client.println ("<form id=\"uploadbanner\" \
 enctype=\"multipart/form-data\" method=\"post\" action=\"upload.php\">");

	    client.println ("<input id=\"fileupload\" \
 name=\"mydata\" type=\"file\" />");

	    client.println ("<input type=\"submit\" \
 value=\"submit\" id=\"submitf\" />");
	    client.println ("</form>");
#endif
#endif		// DO_UPLOAD_FORM

	    /***** end of forms ********/

    
	    client.println ();
            break;
          } else {
	    Serial.println (currentLine);
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
	    else {
	      
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
