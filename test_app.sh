#\!/bin/bash
# Run the app and simulate frame creation
./cubit-quick.app/Contents/MacOS/cubit-quick 2>&1 &
APP_PID=$\!
sleep 3
# Wait a bit then kill the app
sleep 10
kill $APP_PID 2>/dev/null
wait $APP_PID 2>/dev/null
