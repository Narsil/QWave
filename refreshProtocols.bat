@ECHO OFF
  ::
  ::  You neet to specify the absolute path to your
  ::  protoc.exe
  :: Example:
  ::      set PROTOCOMPILER=D:\work\GoogleCode\protobuf\src\vsprojects\Release\protoc.exe
  ::
  set PROTOCOMPILER=D:\work\GoogleCode\protobuf\src\vsprojects\Release\protoc.exe
  ::
  ::
  ::
  set ROOTDIR=%CD%
  set COREDIR=%ROOTDIR%\core\protocol
  set SERVERDIR=%ROOTDIR%\waveserver\protocol
  ::
  :: We have two Include directories since the server/protocol file
  references
  :: protocols under the /core/protocol
  ::
  FOR  /f %%a  IN ('dir %COREDIR%\*.proto /b') DO CALL :process
  %COREDIR%\%%a  %COREDIR% %COREDIR% %COREDIR%
  FOR  /f %%a  IN ('dir %SERVERDIR%\*.proto /b') DO CALL :process
  %SERVERDIR%\%%a  %SERVERDIR% %SERVERDIR% %COREDIR%
  goto :eof

  :process
  set protofile=%1
  set outputdir=%2
  set includedirA=%3
  set includedirB=%4
  echo Processing %protofile%
  ::
  :: Display the compile so any errors will appear.
  :: A successful compile will return nothing
  ::
  @echo ON
  @%PROTOCOMPILER% --cpp_out %outputdir% -I%includedirA%
  -I%includedirB% %protofile%
  @echo OFF
  goto :eof