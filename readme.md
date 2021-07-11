### Hard Drive APM Command Line Utility

------

core code in this project was basically taken from awesome GUI based software **[CrystalDiskInfo](https://github.com/hiyohiyo/CrystalDiskInfo)**. 

but you know what a cmdline utility should task for.

build with msys2 (mingw64) with cmake or Clion.

----

```
Usage:
    apm_set.exe [options]

Options:
    -i <...>    physical drive index: \\.\PhysicalDrive[X]
    -r          read apm state and value only (default)
    -e <...>    enable apm with value: 1 (minimal power) - 254 (max performance)
    -d          disable apm
    -h          print this help
```

to get index of specified physical hard drive, use command `wmic diskdrive list`.

-----

**USE WITH CAUTION AT OWN RISK. NO WARRANTY.**

