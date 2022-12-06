#if 0

This code runs on two different computers simultaneously,
the Arduino and some machine running a browser.  The Arduino
side does three things, it serves a web page to the browser,
which allows the user to set various parameters, it reads
the parameters, and it interprets the parameters to control
the servos.  The broswer side runs a web page with the controls.
The Arduino side is coded in a subset of C++; the browser side
is in a mix of Javascript and HTML.  In the code below,
srgument strings to client.print () calls are the HTML and
Javascript being served to the browser.
					     
#endif

#include <SPI.h>
#include <SD.h>
#include <WiFiNINA.h>
#include <JOAAT.h>

/********** global variables ***********/

/* An instantiation of a class that generates hashes */
JOAAT joaat;

/* An instantiation of a class that turns the Arduino into a web server */
WiFiServer server(80);

/* WiFi credentials */
#define SECRET_SSID "NETGEAR80"
#define SECRET_PASS "magical574"
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password 

/* holds th local ip address.  not sure i need it... */
IPAddress ip;

/******* parameter labels *******/

#define ETY(v) #v
const char * lbls[] = {
#include "lbls.h"
};
size_t lbl_cnt = sizeof(lbls)/sizeof(char *);

/******* parameter enums *******/

#define ETY(v) PARM_ ## v
enum {
#include "lbls.h"
};

/*****
      One of my rules is to never enter the same information more than once
      --it's too easy for the different versions to get out of sync.  What the
      above does is define the macro ETY two different ways and then include a
      a file containing a list of parameter names for each of those ways.
      Under the first definition, that file expands into a char * vector of
      the names; under the second def it expands into a list of indices into
      that vactor, and both lists are guaranteed to stay in sync.

      A macro, BTW, (like where it says "#define ETY(v) #v" above) is a string-
      manipulation thing.  The compiler works in a lot of different stages, but
      the first one goes through the code looking for things like #define
      statements and then, when it sees a string in the code that matches a
      definition, ETY in this case, it plugs the argument, v in this case, into
      the expansion ("#v" or "PARM_ ## v" in these cases) so entries in lbl.h,
      e,g., ETY(pdx), get expanded into "pdx" or PARM_pdx respectively.  (The
      actual expansion process is pretty complex--I won't go into it here.)
      (The preprocessing stage also handles things like #if statements,
      comment removal, and so on--a whole lot of stuff.)
 *****/

/******* parameter structure *********/
/* collected information about each parameter, it's name, it's current value
   and a hash value which will be explaned below.*/

typedef struct {
  double val;
  uint32_t hash;
  const char *name;
} parm_s;

/* A pointer to an unallocated array of the parameters. */
parm_s *parms = nullptr;


/******** subroutines and functions *********/


/********* comparison function for qsort (see below) **********/

int cmp_parm (void *s1, void * s2)
{
  uint32_t a = ((parm_s *)s1)->hash;
  uint32_t b = ((parm_s *)s2)->hash;
  return (a == b) ? 0
    : ((a < b) ? -1 : 1);
}


/********* comparison function for bsearch (see below) **********/

int cmp_parm_str (const void *s1, const void *s2)
{
  union {
    const void *v;
    struct {
      uint32_t m;
      uint32_t l;
    }u;
  } a_u;
  a_u.v = s1;
  uint32_t a = a_u.u.m;
  uint32_t b = ((parm_s *)s2)->hash;
  
  return (a == b) ? 0
    : ((a < b) ? -1 : 1);
}


/********* utility function for qsort (see below) **********/

void swap(void* v1, void* v2, int size)
{
  char buffer[size];
  memcpy(buffer, v1, size);
  memcpy(v1, v2, size);
  memcpy(v2, buffer, size);
}


#if 0

All the control parameters have names like "pdx."  In order to access or
change the value of a parameter, it has to be looked up in the parameter
array, which means matching the string containing the name against the names
in the parameter array.  The "unsophisticated" programmer usually does this
with a series of "if name = this, then do ..., else if name = that..." etc.
Comparing strings is very slow and if you have a lot of names in the array
all these compares take a linearly long time.  Comparing numbers is vastly
faster so the trick is to turn name strings into signature numbers.  This is
called "hashing."

Further, there are faster ways of looking up numbers than by scanning a list,
one of which is called a binary search.  If the list is sorted by hash number
then, if you start with a first compare against the middle of the list, you
can tell immediately whether the target hash number is greater or less than
what you're looking for so you can then skip immediately to either the quarter-
way point on the list, or the three-quarter, and keep doing that recursively
until you find the target.  This is an O(log n) process, i.e., much faster than
a linear search, but it requires the array of targets to be sorted.  qsort,
below is the algo to do the sorting, as seen in action later on.

#endif

void qsort(void* v, int size, int left, int right,
      int (*comp)(void*, void*))
{
  void *vt, *v3;
  int i, last, mid = (left + right) / 2;
  if (left >= right)
    return;

  void* vl = (char*)(v + (left * size));
  void* vr = (char*)(v + (mid * size));
  swap(vl, vr, size);
  last = left;
  for (i = left + 1; i <= right; i++) {
    vt = (char*)(v + (i * size));
    if ((*comp)(vl, vt) > 0) {
      ++last;
      v3 = (char*)(v + (last * size));
      swap(vt, v3, size);
    }
  }
  v3 = (char*)(v + (last * size));
  swap(vl, v3, size);
  qsort(v, size, left, last - 1, comp);
  qsort(v, size, last + 1, right, comp);
}	

#if 0

The other half of the process is the binary search of the sorted list, as done
in bsearch, also seen in use later.

#endif

void *bsearch (const void *key, const void *base0,
	       size_t nmemb, size_t size,
	       int (*compar)(const void *, const void *))
{
  const char *base = (const char *) base0;
  int lim, cmp;
  const void *p;

  for (lim = nmemb; lim != 0; lim >>= 1) {
    p = base + (lim >> 1) * size;
    cmp = (*compar)(key, p);
    if (cmp == 0)
      return (void *)p;
    if (cmp > 0) {	/* key > p: move right */
      base = (const char *)p + size;
      lim--;
    } /* else move left */
  }
  return (NULL);
}


/* a utility Javascript function that sets the values of HTML entities in
the browser with the current parameter values known to the Arduino. */

void initter (WiFiClient client, int idx)
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

/* The long, hairy, process of building the web page */

void buildPage (WiFiClient client)
{
  
	    /***** scripts *******/

/* scripts are Javascript routines that do stuff */

  client.println ("<script>");

  
  /********** showEditor *****/

/* formats parameter data into a decent human-readable form and lets the
user edit it.  The edited result can be saved for later use as a again as
a phantom-specific behaviour. */

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


  
  /********** window.onload *****/

/* immediately the web page is built, initialise the values of HTML entities */

  client.println ("window.onload = function () {");
  client.println ("  let srch = window.location.search;");
  client.println ("  const searchParams = \
 new URLSearchParams(srch);");

  for (int i = 0; i < lbl_cnt; i++)
    initter (client, i);

  client.println ("}"); // end window.onload function



  /********** updateParam *****/

/* send data from the web page to the Arguino */
  
  client.println ("function updateParam(el) {");

  client.println ("const XHR = new XMLHttpRequest();");

  client.println ("  var text = window.location.origin + \"?update=\" + \
el.id + \"=\" + el.value;");
  client.println ("XHR.open('POST', text);");
  client.println ("XHR.setRequestHeader('Content-Type', 'text/plain');");
  client.println ("XHR.send();");

  client.println ("}");


	    /**** function keyHandler(el) ****/

/* The HTML mostly consists of numeric input boxes that can have up/down
arrows clicked, or numbers pounded in followed by an "enter" keypress.  This
handles the enter key. */

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

/* styles control what the HTML looks like */

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


/* the actual page layout */

	    /*** begin page ****/
	    
  client.println ("<h1>Stewart</h2>");
	    
	    
	    /****  forms ****/

/* A couple of truly hairy macros that creae the numeric input boxes.
DO NOT FIDDLE WITH THESE!  You'll wind up tearing your hair out with
frustration and you're to young to be bald. */

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



/* Javascript to send editor data to the Arduino */

  client.println ("<script>");
  client.println ("function saveText() {");

  client.println ("const XHR = new XMLHttpRequest();");

  client.println ("  var text = window.location.origin + \"?text=\" + \
document.getElementById('editor').value;");
  client.println ("XHR.open('POST', text);");
  client.println ("XHR.setRequestHeader('Content-Type', 'text/plain');");
  client.println ("XHR.send();");

  client.println ("}");
 
/* Javascript to just close the editor window */

  client.println ("function abandonText() {");
  client.println ("  window.location.href = window.location.origin;");
  client.println ("  return false;");
  client.println ("}");
  client.println ("</script>");


/* code to put the editor window and controls up */

   client.println ("<div class=\"popup\" \
onclick=\"showEditor()\">Open editor");

  client.println ("<span class=\"popuptext\" id=\"myPopup\" \
style=\"width:560px\">");

  client.println ("<textarea id=\"editor\" rows=\"9\" cols=\"20\" \
    method=\"post\" name=\"george\" wrap=\"off\" \
style=\"width:512px;minWidth=512px;\
height=360px;minHeight=180px;\
background-color=lightgrey\"></textarea><br>");

  client.println ("<button type=\"button\" \
onclick=\"abandonText();\">Abandon</button>");

  client.println ("<button type=\"button\" \
onclick=\"saveText();\">Save</button>");

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

#if 0

Normal C/C++ code has one entry point, called main(), where execution starts.
Arduino code as two such entry points, setup() and loop().  setup() is run once
to, guess what, set things up.  loop(), guess what again, loops forever.

#endif

void setup()
{

  int status = WL_IDLE_STATUS;

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

#if 0
  if (!SD.begin(4)) {
    Serial.println("SD initialization failed!");
    while (1);
  }
#endif

/* create the parameter array and compute the hash values */

  parms = (parm_s *)malloc (sizeof(parm_s) * lbl_cnt);
  for (int i = 0; i < lbl_cnt; i++) {
    parms[i].val  = 0.0;
    parms[i].hash = joaat.encode_str (JOAAT_STR (lbls[i]));
    parms[i].name = lbls[i];
  }

/* sort the array by hash value */

  qsort (parms, sizeof(parm_s), 0, lbl_cnt - 1, cmp_parm);

}


              
void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    // make a String to hold incoming data from the client
    String currentLine = "";
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
	
        char c = client.read();     // read a byte, then
	//	Serial.print (c);
        if (c == '\n') {            // if the byte is a newline character
          if (currentLine.length() == 0) {
	    // if it's blank line, no more input and go ahead and build the
	    // page.
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
	      // if it's incoming data, figure out what kind of data it is...
	      String startString = "update=";
	      int startPos = currentLine.indexOf(startString);
	      if (-1 != startPos) {
		// if it's parameter update data...
		startPos += startString.length ();
		String endString = "HTTP";
		int endPos = currentLine.indexOf(endString);
		if (-1 != endPos) {
		  // isolate the actual data
		  endPos--;
		  String text = currentLine.substring (startPos, endPos);
		  // and then split up up into parameter name
		  // and parameter value
		  int endPos =  text.indexOf ("=");
		  String vbl = text.substring (0, endPos);
		  String vals = text.substring (1 + endPos);
		  double val = vals.toDouble ();

		  // compute the hash of the name...
		  uint32_t hash = joaat.encode_str (JOAAT_STR (vbl.c_str ()));
		  // and search for it
		  void *res = bsearch (reinterpret_cast<void *>(hash), parms,
			 lbl_cnt, sizeof(parm_s), cmp_parm_str);
		  if (res != nullptr) {
		    // if you find it, update the value
		    ((parm_s *)res)->val = val;
		    Serial.println (vbl + " = "
				    + String (((parm_s *)res)->val, 2));
		  }
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
