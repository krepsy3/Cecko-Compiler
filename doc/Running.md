# Running Cecko

## Command-line

The resulting executables are located in `<build-folder>/main` and invoked like:

```bash
cd <build-folder>
<build-folder>/main/cecko6 <cecko-folder>/test/test1.c
```

## Visual Studio 2019

Open *DEBUG -> Debug and Launch Settings for ...* (launch.vs.json) and add the following settings into the configuration(s) in use:
```json
      "currentDir": ".",
      "args": [ "test\\test1.c" ]
```

## Visual Studio Code





