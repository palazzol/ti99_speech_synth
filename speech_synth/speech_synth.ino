
#include <string.h>

// Pin definitions
int DATA_pin[] = {2,3,4,5,6,7,8,9};
int A_pin = 10;
int B_pin = 11;
int G1_pin = 12;
int G2B_pin = A0;
int RDY_pin = A1;

int LED_pin = 13;

// Commands
int speak[] = {0,1,0,1,0,0,0,0};
int addr[] = {0,1,0,0,0,0,0,0};
int reset[] = {1,1,1,1,1,1,1,1};
int readbyte_cmd[] = {0,0,0,1,0,0,0,0};
  
void setup() {      
  for (int i = 0;i<8;i++)
  {
    pinMode(DATA_pin[i], OUTPUT);
  }
  pinMode(G1_pin, OUTPUT);       
  pinMode(A_pin, OUTPUT);       
  pinMode(G2B_pin, OUTPUT);       
  pinMode(LED_pin, OUTPUT);       
  pinMode(B_pin, OUTPUT);       
  pinMode(RDY_pin, INPUT_PULLUP);    

// OFF
  digitalWrite(G1_pin,LOW); 
  digitalWrite(G2B_pin,LOW); 
  digitalWrite(A_pin,LOW); 
  digitalWrite(B_pin,HIGH);

  Serial.begin(115200);
  Serial.println("TI speech synth code");
  //write_rst();
  //write_addr(0);
  //Serial.println("reset");

  do_reset_command();
    
  wait_for_ready();

    //do_load_address(9);
    //do_load_address(14);
    //do_load_address(7);
    //do_load_address(7);
    //do_load_address(0);
}

void loop() {
    print_ready_status();

    delay(10000);
    
    /*
    // Speak one sample
    do_load_address(9);
    do_load_address(14);
    do_load_address(7);
    do_load_address(7);
    do_load_address(0);
    do_speak_command();
    */

    // Match a String
    do_speak_entry("TEXAS INSTRUMENTS");
    
    /*
    // Dump the ROM
    int b = 0;
    for(int i=0; i<2048; i++)
    { 
      for(int j=0;j<16;j++) {
        b = do_readbyte_command();
        if (b<16)
          Serial.print('0');
        Serial.print(b,HEX);
      }
      Serial.println();
    }
    */
}

void print_ready_status()
{
  if(is_ready())
  {
    //Serial.println("ready");
    digitalWrite(LED_pin,HIGH);
  }
  else
  {
    //Serial.println("not ready");
    digitalWrite(LED_pin,LOW);
  }
}

// Helper functions

void wait_for_ready()
{
  while(!is_ready());
}

void wait_for_not_ready()
{
  while(is_ready());
}

void enable_WS()
{
  // Enable Write
  digitalWrite(A_pin,HIGH);
  digitalWrite(B_pin,HIGH);
  digitalWrite(G1_pin,HIGH);
}

void disable_WS()
{
  digitalWrite(G1_pin,LOW);
}

void enable_RS()
{
  // Enable Read
  digitalWrite(A_pin,LOW);
  digitalWrite(B_pin,HIGH);
  digitalWrite(G1_pin,HIGH);
}

void disable_RS()
{
  digitalWrite(G1_pin,LOW);
}

void data_write(int data[])
{
  for (int i = 0;i<8;i++)
  {
    digitalWrite(DATA_pin[i], data[i]);
    pinMode(DATA_pin[i], OUTPUT);
  }
}

void data_tristate()
{
  for (int i = 0;i<8;i++)
  {
    pinMode(DATA_pin[i], INPUT);
  }
}

int data_read()
{
  int rv = 0;
  for (int i = 0;i<8;i++)
  {
    rv <<= 1;
    rv |= digitalRead(DATA_pin[i]);
  }
  return rv;
}

//////

struct Node
{
  int name_len;
  char name[32];
  int prev_ptr;
  int next_ptr;
  int data_ptr;
};

void do_read_node(struct Node *node, int address)
{
  //Serial.println(address);
  
  do_load_address(address & 0x0f);
  do_load_address((address>>4) & 0x0f);
  do_load_address((address>>8) & 0x0f);
  do_load_address((address>>12) & 0x0f);
  do_load_address(0);
  //delay(10);
  node->name_len = do_readbyte_command();
  //Serial.println(node->name_len);
  
  int i;
  for(i=0;i<node->name_len;i++)
    node->name[i] = do_readbyte_command();
  node->name[i] = 0;

  Serial.println((char *)node->name);
  
  node->prev_ptr = do_readbyte_command()<<8;
  node->prev_ptr += do_readbyte_command();

  node->next_ptr = do_readbyte_command()<<8;
  node->next_ptr += do_readbyte_command();

  do_readbyte_command();

  node->data_ptr = do_readbyte_command()<<8;
  node->data_ptr += do_readbyte_command();  

  int data_len = do_readbyte_command(); // Unused
}

int do_speak_tree(char *s, int address)
{
  Node node;
  
  do_read_node(&node, address);

  int x = strncmp(s, node.name, node.name_len);
  if (x == 0) {
    do_load_address(node.data_ptr & 0x0f);
    do_load_address((node.data_ptr>>4) & 0x0f);
    do_load_address((node.data_ptr>>8) & 0x0f);
    do_load_address((node.data_ptr>>12) & 0x0f);
    do_load_address(0);
    do_speak_command(); 
    return 0;
  } else if (x < 0) {
    if (node.prev_ptr == 0)
      return -1;
    return do_speak_tree(s, node.prev_ptr);
  } else {
    if (node.next_ptr == 0)
      return -1;
    return do_speak_tree(s, node.next_ptr);
  }
}
  
void do_speak_entry(char *s)
{
  int address = 1;
  int rv = do_speak_tree(s, address);
  if (rv == 0)
    Serial.println("Speech OK");
  else
    Serial.println("Speech not found");
}

// Commands

void do_load_address(int addre)
{
  //Serial.print("do_load_address(");
  //Serial.print(addre);
  //Serial.println(")");
  
  addr[4] = (addre>>3)&0x01;
  addr[5] = (addre>>2)&0x01;
  addr[6] = (addre>>1)&0x01;
  addr[7] = (addre>>0)&0x01;

  data_write(addr);

  enable_WS();
  
  wait_for_not_ready();

  disable_WS();

  wait_for_ready();

  data_tristate();
}

void do_speak_command()
{
  data_write(speak);

  enable_WS();
  
  wait_for_not_ready();

  disable_WS();

  wait_for_ready();

  data_tristate();
}

void do_reset_command()
{
  data_write(reset);

  enable_WS();

  //Serial.println("a");
  
  //wait_for_not_ready();

  //Serial.println("b");

  disable_WS();

  //Serial.println("c");

  wait_for_ready();

  data_tristate();
  
  //Serial.println("d");
}

int do_readbyte_command()
{
  //Serial.println("do_readbyte_command()");
  
  data_write(readbyte_cmd);

  enable_WS();
  
  wait_for_not_ready();

  disable_WS();

  wait_for_ready();

  data_tristate();

  enable_RS();

  // Special
  wait_for_ready();

  int b = data_read();

  disable_RS();

  return b;
}

boolean is_ready()
{
  if(digitalRead(RDY_pin))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
