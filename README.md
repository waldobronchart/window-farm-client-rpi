

## Build with
`make target=raspberry`

## Run with
`sudo ./window_farm_client --token=TOKEN_HERE`

## Some things for me to remember

Tip: error while loading shared Libraries: liblog4cplus-1.1.so.9: CAN not shared Open Object File: No SUCH File or Directory

```
　　find / -name liblog4cplus-1.1.so.9 / * I found in the /usr/local/lib/liblog4cplus-1.1.so.9*/

　　cd / etc 
　　sudo gedit ld.so.conf (add /usr/local/lib/) 
　　sudo ldconfig
```

Also, Virtual Pins can react to value updates and requests.

For example, this function will be called every time Widget in the Blynk App requests data for Virtual Pin 5:

BLYNK_READ(5)
{
  // Usually, you will need to respond with a virtual pin value.
  Blynk.virtualWrite(5, some_value);
}
This function will be called every time Widget in the Blynk App writes value to Virtual Pin 1:

BLYNK_WRITE(1)
{
  BLYNK_LOG("Got a value: %s", param.asStr());
  // You can also use: param.asInt() and param.asDouble()
}
BLYNK_READ/BLYNK_WRITE functions are effectively "getters/setters" of the Virtual Pins if you're familiar with this concept in other programming languages.
Please also take into account that these functions should take minimal time to execute, so avoid using sleep/delay inside of them.
