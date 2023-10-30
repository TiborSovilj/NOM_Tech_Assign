////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		mqtt_cert.c
 *	@author   	Tibor Sovilj
 *	@date		26.10.2023
 *	@version	V1.0.0

 *	@brief 		Source file for MQTT TLS certificate. File contains definition
                for getter function that provides mqtt lib with certificate
                string which is hardcoded in this particular case.
                
                More info on reasoning behing this implementation is on the 
                following link:

                https://github.com/espressif/esp-idf/issues/5177 
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *  @addtogroup MQTT_CERT_CONFIG
 *  @{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
// CONFIG Variables and Tables
////////////////////////////////////////////////////////////////////////////////

/**
 *  Hardcoded certificate string containing client key, client certificate and 
 *  server certificate.
 */
const char* gp_mqtt_certificate= "-----BEGIN RSA PRIVATE KEY-----\n\
MIIEowIBAAKCAQEAsgyMKg+og7h7AhVoMvQDXDyA1LrWHO2BTrJ6HdARLqsh9QP1\n\
OatY05b40092Uc1BMkOTlj+6bug6qIRYpZi6VrGmtCwQ+RASWiYIhbfTEidPvu8F\n\
a4/gQmQZbcmV4qwvDB/8AUUN68Me5eRQHQNFbHMmqJhrVJ140bSrrH9B9GlXCR39\n\
fbHk2rlYe4vzr98jSuvWp1GItSLtMGWidvTguEJFsSRcUoFo0sJ+68IOyOXcEDGk\n\
r4vJFGogNB8QyaVLIXa7JWKgIIgmVjJvYDaePQwUXePvGjaZzRfR89fPrr0S/CVh\n\
LDGrb0WHYiC+RivlCJZWEoODbKfX+8moh7r60wIDAQABAoIBAFu1EsznW8jc0J72\n\
H8F+5ewwTbtEsNwdiSjbzQJmFTOQeeEVtM2LcCkr7eYJW8wuiJI3NGGDWaeeffgm\n\
kvJYhEH7Jv5OZD+lA47jYChf0pvbG7wgqQ4KAVyw6lgeKNGkFmeMYeTViKIS0mte\n\
+dS3xp5hgVv9hesDYSPCh1pGAda9P1alRRlWC2OJk3769vs4nwOu03IGQOSWBl4Y\n\
/QSrU5lLgU2v1CtieZQRENkpYyBZiOBO88thskjok99dqKAhDAgd9N3/sa1Nw7Gw\n\
tMxqeQAoSG0PCk4jIv5gjE8LHROrv4urpdfqIBOXGttbK6dg6IPqbwSOhaYgtSR6\n\
5Qj6VqECgYEA2kCROapJS3iIsQrMHZkOyNj+NCZfDW809g24biPkVOtpZJv/ngEi\n\
c9UhSmGNcOMtI+NGxWvAl9xmTw7b8xzOk+ITnCOBoKjDFQ0DSehaKGMbNW5RbGN2\n\
gpRZVOJ270FyIzAXyDCCGKJQW5fDlTFCXqEr1oUVC3kMsqlyCVh9IWUCgYEA0Nfs\n\
3ogP5KIwuNlELK4RpfpNUa8TCiGQ0D6ezUNB3ozgcYAJZa8WS+meYIogvrT/e4Vo\n\
L8mXdTX9trGNMeUwao0PmbbTMqLR5E19HqJd1HKEmXzwZ0Be8scFNdAxMEdeBgmM\n\
ih9E+Z/a8ViPhlAu5xtXyhgVLMaSDPvTWSQkw9cCgYAmqsfP+p5vs5QsIaiWGdbn\n\
uKIY5S9z9t7gNQAW6175uJd8jrLT8ImFEh6KygvAE0+dCxgvw+5kOVUa7pwDT3g9\n\
9RDaWeQObbfaU+rgPj0y6JQafEgKtvh5HAVTp6fArcyl9VBRVF7INIGeKJ4rIYYL\n\
s+xLXlqjJLgeMy0UAMxyjQKBgGYQsQ3Ml1/YuFEOtdfUNoHUg0chdf+kid6MTBXr\n\
Ad0fIm218mHEoPP2t9VcjEZHtPiMKW/5aND60wUfXu78oJ3iVLZ9+Fet5UBbcoOv\n\
PIYgdZeBzQfZGM4z3+L93ZxHtLbkoc+7Gn2Y12rOKk6tD08ZON1myap5XVWFGTRe\n\
iq0/AoGBAKQ5LGbR0ioZUv859OyjHyyP3qRHvvxzShe5jX65aDI1mKWjL7DpmwEb\n\
tj+oALX1pGStlSsOp3dmdYY7hcqPsMi+y9gOS2Go6dwUuSOCt2ldjLa5+7/ZF4DD\n\
68v7hYBw4OpVCvby/apMCu9yAMOj9mbHpCXMpMgUMQN0lr4e5cOn\n\
-----END RSA PRIVATE KEY-----\n\
-----BEGIN CERTIFICATE-----\n\
MIIDVDCCAjygAwIBAgIBADANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCR0Ix\n\
FzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTESMBAGA1UE\n\
CgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVpdHRvLm9y\n\
ZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzAeFw0yMzEwMjYxNzE2\n\
NTdaFw0yNDAxMjQxNzE2NTdaMC4xCzAJBgNVBAYTAlNJMQ8wDQYDVQQKDAZOb21u\n\
aW8xDjAMBgNVBAMMBVRJQk9SMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n\
AQEAsgyMKg+og7h7AhVoMvQDXDyA1LrWHO2BTrJ6HdARLqsh9QP1OatY05b40092\n\
Uc1BMkOTlj+6bug6qIRYpZi6VrGmtCwQ+RASWiYIhbfTEidPvu8Fa4/gQmQZbcmV\n\
4qwvDB/8AUUN68Me5eRQHQNFbHMmqJhrVJ140bSrrH9B9GlXCR39fbHk2rlYe4vz\n\
r98jSuvWp1GItSLtMGWidvTguEJFsSRcUoFo0sJ+68IOyOXcEDGkr4vJFGogNB8Q\n\
yaVLIXa7JWKgIIgmVjJvYDaePQwUXePvGjaZzRfR89fPrr0S/CVhLDGrb0WHYiC+\n\
RivlCJZWEoODbKfX+8moh7r60wIDAQABoxowGDAJBgNVHRMEAjAAMAsGA1UdDwQE\n\
AwIF4DANBgkqhkiG9w0BAQsFAAOCAQEAkrsJrzSl9zK3qU3bYC3GCQMK0TSezU5A\n\
kttxOJ3EXCjjAtWbHGXGbSI+gqbLWFpSpm7ssJEgtnYMH1+73xOmPJ5kClk6GB7a\n\
Oki7dppM+AMrPBRpDIYWqYxusI/jZDZk4RTk9EVoUAv4IOP1AMk975qEmOqKg2E4\n\
IUWZJ7GcmTZdrudGlap/FbKaFvO7vqwWMMmAP03q6i2kWR90EBtRVmhEoCCTaMkc\n\
Qzw4i2lTApTcxtk22YArbbGsAX3xSjB3pF599VQoVtCvMKHrhx7T9VclIGTVlPSW\n\
jLdIr+tUehOybEfcyQ2y8pDkPUXtOEyvtDYaN+LalVV+Mn+KDReGZQ==\n\
-----END CERTIFICATE-----\n\
-----BEGIN CERTIFICATE-----\n\
MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL\n\
BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG\n\
A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU\n\
BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv\n\
by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE\n\
BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES\n\
MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp\n\
dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ\n\
KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg\n\
UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW\n\
Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA\n\
s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH\n\
3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo\n\
E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT\n\
MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV\n\
6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\n\
BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC\n\
6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf\n\
+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK\n\
sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839\n\
LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE\n\
m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=\n\
-----END CERTIFICATE-----\n";

////////////////////////////////////////////////////////////////////////////////
// CONFIG Functions definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief   Getter function for hardcoded certificate string.
 * 
 * @return  Certificate string.
 */
////////////////////////////////////////////////////////////////////////////////
const char* mqtt_get_certificate (void)
{
    return gp_mqtt_certificate;
}

////////////////////////////////////////////////////////////////////////////////
/**
 *  @} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

