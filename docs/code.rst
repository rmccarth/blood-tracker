.. _Code:

Sourcecode
^^^^^^^^^^

We'll start by covering the AWS Lambda Function and then move onto the Particle Web IDE code. Thats right, leveraging the cloud means we only have to have
code running in two places for this project!

.. code-block:: python
   :linenos:

    import json
    import requests

    API_TOKEN="MZd27P3S1YaQhWhkpaZ"

    def lambda_handler(event, context):
        
        allMachines = getRequest("/machines/get/all", API_TOKEN).json()
        mostRecentMachineID = allMachines[len(allMachines)-1]['id']
        
        return {
            'statusCode': 200,
            'id': json.dumps(mostRecentMachineID)
        }

    def getRequest(url: str, apitoken: str) -> str:
        HEADERS = {"User-Agent": "blood-tracker-api"}
        BASE = "https://www.hackthebox.eu/api"
        return requests.get(f"{BASE}{url}?api_token={apitoken}", headers=HEADERS)



The first method "lambda_handler" is an AWS default function that listens for calls to it. We 
can leave the method definition as it is, and adjust the code within it to execute our custom logic.

.. code-block:: python
   :linenos:

    import json
    import requests

    API_TOKEN="MZd27P3S1YaQhWhkpaZ"

    def lambda_handler(event, context):

The lambda_handler first calls the getRequest function, which essentially just formats our request (note the
critical HEADER value container a User-Agent string. Without this User-Agent, the HTB API will reject the request.
The getRequest method returns a response object from the HTB API, taking in the formatting provided by "/machines/get/all" and the API_TOKEN value.

.. code-block:: python
   :linenos:

   allMachines = getRequest("/machines/get/all", API_TOKEN).json()

   def getRequest(url: str, apitoken: str) -> str:
        HEADERS = {"User-Agent": "blood-tracker-api"}
        BASE = "https://www.hackthebox.eu/api"
        return requests.get(f"{BASE}{url}?api_token={apitoken}", headers=HEADERS)

Next, the response from the HTB API is parsed to retrieve the most recently released boxes 'id' value.
Since the HTB API puts the machines/get/all list in cronological format, we can rely on that to simply get the last
dictionary in the array, and select the 'id' value from that dictionary. 

.. code-block:: python
   :linenos:

   mostRecentMachineID = allMachines[len(allMachines)-1]['id']

Finally, the lambda_handler returns a 200 status code, and formats the mostRecentMachineID as valid JSON with json.dumps().

.. code-block:: python
   :linenos:

    return {
            'statusCode': 200,
            'id': json.dumps(mostRecentMachineID)
        }


.. topic:: Onto the htb_monitor.ino code!
   
   The following is the full sourcecode. Feel free to copy-and-paste into your Particle Web IDE.
   Scroll to the bottom for a step-by-step walkthrough of how the code works.

.. code-block:: c++
   :linenos:

    #include <TM1637Display.h>
    #define CLK D2//pins definitions for TM1637 and can be changed to other ports
    #define DIO D3

    TM1637Display display = TM1637Display(CLK, DIO);

    int global_boxid = 281;

    void setup() {
        
        Particle.subscribe("hook-response/release-id", idHandler, MY_DEVICES);
        Particle.subscribe("hook-response/user-owns", userHandler, MY_DEVICES);
        Particle.subscribe("hook-response/root-owns", rootHandler, MY_DEVICES);
        display.setBrightness(0xff);
    }

    void loop() {
            
            if (Time.weekday() == 1) {
                if (Time.hour() == 7) {
                    if ((Time.minute() == 0) || (Time.minute() == 1)) {
                        String getRecentBoxID = "getRecentBoxID";
                        Particle.publish("release-id", getRecentBoxID, PRIVATE);
                    }
                }
            }
            
            String boxid = String(global_boxid);
            Particle.publish("user-owns", boxid, PRIVATE);
            delay(10000);
            Particle.publish("root-owns", boxid, PRIVATE);
            delay(10000);

    }

    void idHandler(const char *event, const char *data) {
        // Handle the integration response
        
        global_boxid = String(data).toInt();
        
    }

    void userHandler(const char *event, const char *data) {
        // Handle the integration response
      
        const uint8_t user[] = {
            SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // U
            SEG_A | SEG_G | SEG_D | SEG_C | SEG_F,   // S
            SEG_A | SEG_G | SEG_D | SEG_E | SEG_F,   // E
            SEG_A | SEG_B | SEG_F | SEG_E            // R

        };
        
        display.setSegments(user);
        delay(10000);
        
        int user_count = String(data).toInt();
        display.showNumberDec(user_count, false, 4, 0);
        
    }

    void rootHandler(const char *event, const char *data) {
        // Handle the integration response
        
        const uint8_t root[] = {
            SEG_A | SEG_B | SEG_F | SEG_E,                   // R
            SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
            SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
            SEG_A | SEG_B | SEG_C                            // T
        };
        
        display.setSegments(root);
        delay(10000);
        
        int root_count = String(data).toInt();
        display.showNumberDec(root_count, false, 4, 0);
        
    }


The following code block includes the TM1637Display library which we will need to 
control the "clock" display. The nice thing about this library is it allows us to have fine-grained
control over what exactly is displayed in the various clock "segments" which are labeled SEG_A-G, in clockwise fashion
and with "SEG_G" being the center bar. Lines 2 and 3 define where our clock is plugged into our Particle Argon breadboard.
Feel free to have your plugged into a different location. Mine is plugged into the section labeled "D2".

Finally, the global_boxid is a global integer value that we will change based on the newly released boxes id value
which is retrieved from the Lambda/Gateway API function that we created in AWS.

.. code-block:: c++
   :linenos:

    #include <TM1637Display.h>
    #define CLK D2//pins definitions for TM1637 and can be changed to other ports
    #define DIO D3

    TM1637Display display = TM1637Display(CLK, DIO);

    int global_boxid = 281;


This next "setup" function servers to run code before the main "loop" function of our program.
This code is run to "bootstrap" the rest of the program. Here we will subscribe to our various
webhooks that we have setup in Particle Cloud and provide callback functions (2nd arg) for when we 
receive data from the broker(s). See :ref:`quickstart` for a detailed explanantion
on how to configure the webhooks. 

Finally, we initialize the display to be as bright as possible. Because why would anybody ever
RGB at half intensity?

.. code-block:: c++
   :linenos:

    void setup() {
            
            Particle.subscribe("hook-response/release-id", idHandler, MY_DEVICES);
            Particle.subscribe("hook-response/user-owns", userHandler, MY_DEVICES);
            Particle.subscribe("hook-response/root-owns", rootHandler, MY_DEVICES);
            display.setBrightness(0xff);
        }

The next code block is the main loop of the program. The loop first checks if it is a Saturday at 7:00 or 7:01 pm (UTC default). 
That specific time is when HackTheBox releases new machines. If the time is one of those two possible times,
our Particle device will publish to the Particle Webhook 'release-id'. This will trigger the webhook, and since our Particle device
is subscribed to the webhook in the setup() function, we will be able to retrieve the data.

The data is handled by setting the callback function idHandler inside the Particle.subscribe function.

.. code-block:: c++
   :linenos:

   void loop() {
            
            if (Time.weekday() == 1) {
                if (Time.hour() == 7) {
                    if ((Time.minute() == 0) || (Time.minute() == 1)) {
                        String getRecentBoxID = "getRecentBoxID";
                        Particle.publish("release-id", getRecentBoxID, PRIVATE);
                    }
                }
            }
            
            String boxid = String(global_boxid);
            Particle.publish("user-owns", boxid, PRIVATE);
            delay(10000);
            Particle.publish("root-owns", boxid, PRIVATE);
            delay(10000);

    }

Our idHandler function looks like this:

.. code-block:: c++
   :linenos:

    void idHandler(const char *event, const char *data) {
        // Handle the integration response
        
        global_boxid = String(data).toInt();
        
    }

The idHandler function retrieves the data and converts from a char* array into a String and finally from a String into an int.
I did it this way since the Particle default library contains easy conversions form Strings via the .toInt() function.
The function finally sets that integer value to our global_boxid which is a global value and can be accessed from any subsequent function.

In the event that the current time is NOT 7:00 or 7:01pm on a Saturday, the rest of the loop() will execute. This will take the current
value stored in the global_boxid and pass it to the user-owns and root-owns webhooks in order to execute their respective webhooks (whose callback
functions are again defined in their subscription definition in setup()). 

The delay(10000) instructions tell the Particle device to pause execution for 10000 milliseconds. This is so that we have enough time to read the values
that we are going to write to the display. 

The respective callback functions for the user-owns and root-owns webhooks look like this:

.. code-block:: c++
   :linenos:

    void userHandler(const char *event, const char *data) {
            // Handle the integration response
            
            const uint8_t user[] = {
                SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // U
                SEG_A | SEG_G | SEG_D | SEG_C | SEG_F,   // S
                SEG_A | SEG_G | SEG_D | SEG_E | SEG_F,   // E
                SEG_A | SEG_B | SEG_F | SEG_E            // R

            };
            
            display.setSegments(user);
            delay(10000);
            
            int user_count = String(data).toInt();
            display.showNumberDec(user_count, false, 4, 0);
            
        }

This user-own callback function defines an array of segment values in order to dictate what is written to the display.
The display has 4 areas to be written to. For each area there are 7 possible segments that can be turned on or off.
The array defines which segments in each area will be turned on. The segments are defined in clockwise order around the area,
and the middle bar of the '8' is SEG_G. The top middle bar of the area is SEG_A. The userHandler function takes this user[]
and passes it to the display.setSegments function, which is provided by the library we imported into the Web IDE. This essentially
'lights up' the display at these segment values. The function then waits 10,000 milliseconds before using the display.showNumberDec
function to indicate the number of user-owns found from the query generated by the webhook.

.. code-block:: c++
   :linenos:

    void rootHandler(const char *event, const char *data) {
        // Handle the integration response
        
        const uint8_t root[] = {
            SEG_A | SEG_B | SEG_F | SEG_E,                   // R
            SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
            SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
            SEG_A | SEG_B | SEG_C                            // T
        };
        
        display.setSegments(root);
        delay(10000);
        
        int root_count = String(data).toInt();
        display.showNumberDec(root_count, false, 4, 0);
        
    }

The rootHandler function is exactly the same as the userHandler function except it prints "ROO7" to the device and
then displays the number of root-owns from the query generated by the root-owns webhook in Particle Cloud.
