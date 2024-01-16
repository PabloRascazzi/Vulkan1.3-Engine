using System;

namespace Engine {

    public static class Debug {

        public static void Log(object message) {
            // TODO - internal call to Engine's logger/debugger.
            Console.WriteLine(message);
        }
    }
}