# procclock

A simple `cpulimit`-like for user-interactive stuff.

## Description

Let's say your CPU is fast but you'd like to test a program as if it
was running on way slower CPU.

You may consider `cpulimit` but it comes at a disadvantage: It steals
the time by big blocks. While this makes sense for batch jobs, it has
little use for testing UI interactivity or AV playback.

For this case, we need a tool that allows to specify how much time by
what frequency we'd like to give that program time to run.

How? Just like `cpulimit`, send `SIGSTOP` and `SIGCONT` any when and then. 
No cgroups, no driver hacks, no hardware clock setting, no VM, no sudo.

## Limitations

* Linux only.
* Does not support offspring processes.
* Not extremely precise, see below.
* Not correcting the delay of induced self-time.

## Building

Just compile the only file, like:

```
$ gcc main.c -o procclock
```

## Use

```
$ ./procclock PID TIME_US FREQ
```

With `PID` being the process ID of a running process, `TIME_US` the
time in microseconds that procclock grants to run between continuation
and suspension and `FREQ` the number of times per seconds this is
supposed to happen.

In case of `TIME_US` or `FREQ` being below the resolution of what
`nanosleep` actually is capable of, these values are set to
next-best level.

Use Ctrl + C to stop procclock and leave the process run at full speed
again.

## License

_The Unlicense_, please confer to the `LICENSE` file.
