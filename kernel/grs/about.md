Global registry system

This is a system where each module can import and save configuration based of its value and path
Essentially Windows Registry, This system is storing configuration in memory.
Something like global variables but for modules in kernel.

How does it work:
Each module/file/driver/app can acces the GRS:
create, write, read, destroy (hold, protect should exists in future versions)
Each registry gets its name and value (location in future versions aswell)
Registry are runtime only! They are not persistent and are temporary for dynamic kernel runtime

Why:
It is used to define dynamic global values.

It will be used in near future. I need to implement better version and keep this as a reminder.
