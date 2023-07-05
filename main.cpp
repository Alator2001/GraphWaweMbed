#include "mbed.h"
#include "TCPSocket.h"
#include "MQTTmbed.h"
#include "MQTTClientMbedOs.h"

bool checkFlag = false;

SocketAddress sa;
NetworkInterface *net = NetworkInterface::get_default_instance();
Thread mqtt_send_thread(osPriorityNormal, 1024);
TCPSocket socket;
MQTTClient client(&socket);
SocketAddress a;
char payload[256];

char* topic = "mbed-sample";
char* hostname = "37.75.251.18";
int port = 1883;
float version = 0.6;

Timer t;
InterruptIn sensorHolla(PA_6, PullUp);
DigitalOut led_power(PB_1);
DigitalOut led_send(PB_2);

double period;
int period_sec;

void mqtt_send()
{
    period_sec = period;
    printf("%d\n", period_sec);
    sprintf(payload, "%d", period_sec);
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void *)payload;
    message.payloadlen = strlen(payload);
    int rc = client.publish(topic, message);
    //printf("Publish with rc %d\n\r", rc);
}

void timer_start()
{
    t.stop();
    period = t.read_ms();
    t.reset();
    t.start();
    checkFlag = true;
}



int mqtt_connect()
{
    net->gethostbyname(hostname, &a);
    a.set_port(port);

    printf("Connecting to %s:%d\r\n", hostname, port);
    socket.open(net);
    printf("Opened socket\n\r");
    int rc = socket.connect(a);
    if (rc != 0)
    {
        printf("rc from TCP connect is %d\r\n", rc);
        return 0;
    }
    else
        printf("Connected socket\n\r");

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3; // 3 or 5
    data.clientID.cstring = "2284";
    data.username.cstring = "itacademy";
    data.password.cstring = "student804";

    if ((rc = client.connect(data)) != 0)
    {
        printf("rc from MQTT connect is %d\r\n", rc);
        return 0;
    }
    else
    {
        printf("MQTT client connected\n\r");
        return 1;
    }
}

int main()
{
    led_send = 1;
    led_power = 1;
    if (!net)
    {
        printf("Error! No network inteface found.\r\n");
        return 0;
    }
    printf("\r\nConnecting...\r\n");
    int ret = net->connect();
    if (ret == NSAPI_ERROR_IS_CONNECTED)
    {
        printf("Already connected!\n\r");
    }
    else if (ret != NSAPI_ERROR_OK)
    {
        printf("\r\nConnection error: %d\r\n", ret);
        return -1;
    }

    printf("Success\r\n\r\n");
    printf("MAC: %s\r\n", net->get_mac_address());
    net->get_ip_address(&sa);
    printf("IP: %s\r\n", sa.get_ip_address());
    net->get_netmask(&sa);
    printf("Netmask: %s\r\n", sa.get_ip_address());
    net->get_gateway(&sa);
    printf("Gateway: %s\r\n", sa.get_ip_address());
    printf("\r\nDone\r\n");

    int mqtt_connect_status;
    mqtt_connect_status = mqtt_connect();
    if (mqtt_connect_status == 1)
    {
        led_send = 0;
    }
    sensorHolla.fall(&timer_start);
    
    while (1)
    {
        if (checkFlag == true && period != 0)
        {
            mqtt_send();
            checkFlag = false;
        }
    }
}

