# Creating your Cecko repository

It is advisable to have your work backed up in a private remote repository. If you store it in [gitlab.mff.cuni.cz](https://gitlab.mff.cuni.cz/), your teachers may easily be able to access your sources in case you need help.

Important: **Your remote repository shall remain private**, otherwise you may be deemed guilty if a plagiarism case appears. Grant access only to your teachers, even after your work is finished and points awarded.

Create your private remote repository using the gitlab web interface (initialize it with README.md).

## Importing the skeleton into your private repository

Clone your private repository into a local folder:

```
git clone <private-remote-repository-url> <local-folder>
```

Pull [teaching/nswi098/cecko/skeleton](https://gitlab.mff.cuni.cz/teaching/nswi098/cecko/skeleton) into the local repository:

```
cd <local-folder>
git remote add upstream git@gitlab.mff.cuni.cz:teaching/nswi098/cecko/skeleton.git
git pull upstream master
```

In TortoiseGit, the commands above are done in the **Git Sync...** dialog by first using **Manage** to add the new *remote* called `upstream`, then selecting `upstream` and invoking **Pull**. (Remember to switch back to `origin` if you want to pull from your private repository later.)

_The first upstream pull will likely trigger a conflict on README.md - resolve it in any way you like._

Repeat `git pull upstream master` whenever you think the skeleton repository may have been updated. If you encounter conflicts on subsequent upstream pulls, it probably means that you added your code to an unexpected place in the skeleton files. Try to resolve it carefully, if in doubts, ask your teachers.

If you have more than one local folder, repeat the procedure in all of them but remember to `push` to and `pull` from your private remote repository (as is customary when working with multiple local folders) whenever you switch to another local folder, otherwise you risk conflicts due to upstream pulls performed more than once.

