#ifndef _RAPRAPP
#define _RAPRAPP
// Protolib
#include "protokit.h"
// Rapr
#include "raprLogicTable.h"
// Mgen
#include "mgen.h"

class RaprApp : public ProtoApp, public MgenController
{
public:
  enum CmdType {CMD_INVALID, CMD_ARG, CMD_NOARG};
  static const char* const CMD_LIST[];

  RaprApp();
  virtual ~RaprApp();

  // MgenController virtual method
  void OnMsgReceive(MgenMsg& msg);
  // MgenController virtual method
  void OnOffEvent(char* buffer, int len);
  void OnStopEvent(char* buffer, int len);
  virtual bool OnStartup(int argc, const char*const* argv);
  virtual bool ProcessCommands(int argc, const char*const* argv);
  virtual void OnShutdown();
  void Usage();
  bool OnCommand(const char* cmd, const char* val,BehaviorEvent::EventSource eventSource);

private:
  void OnControlEvent(ProtoSocket& theSocket, ProtoSocket::Event theEvent);
  void OnMgenControlEvent(ProtoSocket& theSocket, ProtoSocket::Event theEvent);
  static CmdType GetCmdType(const char* string);
  char 			filename[512];
  Rapr                   rapr;               // Rapr engine
  bool                   convert;
  ProtoPipe              rapr_control_pipe;  // Remote Control Message Pipe
  bool                   rapr_control_remote;
  Mgen                   mgen;               // Mgen engine
#ifdef HAVE_GPS
  // ljt 11/13/06 - was not in rapr svn repo
  //static bool GetPosition(const void* gpsHandle, GPSPosition& gpsPosition);
  //GPSHandle         gps_handle;
  //GPSHandle         payload_handle;
#endif // HAVE_GPS
}; // end class RaprApp

#endif // _RAPRAPP
