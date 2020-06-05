#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define NTP_TIMESTAMP_DELTA 2208988800ull

#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) 
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) 
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) 

void error( char* msg )
{
    perror( msg ); 

    exit( 0 ); 
}

int main( int argc, char* argv[ ] )
{
  int sockfd, n; 

  int portno = 8080; 

  char* host_name = "127.0.0.1"; 

  typedef struct
  {

    uint8_t li_vn_mode;                  

    uint8_t stratum;        
    uint8_t poll;           
    uint8_t precision;      

    uint32_t rootDelay;     
    uint32_t rootDispersion;
    uint32_t refId;         

    uint32_t refTm_s;       
    uint32_t refTm_f;       

    uint32_t origTm_s;      
    uint32_t origTm_f;      

    uint32_t rxTm_s;        
    uint32_t rxTm_f;        

    uint32_t txTm_s;        
    uint32_t txTm_f;        

  } ntp_packet;             

  ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  memset( &packet, 0, sizeof( ntp_packet ) );

  *( ( char * ) &packet + 0 ) = 0x1b;

  
  struct sockaddr_in serv_addr; 
  struct hostent *server;      

  sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

  if ( sockfd < 0 )
    error( "ERROR opening socket" );

  server = gethostbyname( host_name ); 

  if ( server == NULL )
    error( "ERROR, no such host" );

  bzero( ( char* ) &serv_addr, sizeof( serv_addr ) );

  serv_addr.sin_family = AF_INET;

  bcopy( ( char* )server->h_addr, ( char* ) &serv_addr.sin_addr.s_addr, server->h_length );

  serv_addr.sin_port = htons( portno );

  if ( connect( sockfd, ( struct sockaddr * ) &serv_addr, sizeof( serv_addr) ) < 0 )
    error( "ERROR connecting" );

  n = write( sockfd, ( char* ) &packet, sizeof( ntp_packet ) );

  if ( n < 0 )
    error( "ERROR writing to socket" );

  n = read( sockfd, ( char* ) &packet, sizeof( ntp_packet ) );

  if ( n < 0 )
    error( "ERROR reading from socket" );

  packet.txTm_s = ntohl( packet.txTm_s );
  packet.txTm_f = ntohl( packet.txTm_f );


  time_t txTm = ( time_t ) ( packet.txTm_s - NTP_TIMESTAMP_DELTA );

  printf( "Time: %s", ctime( ( const time_t* ) &txTm ) );

  return 0;
}