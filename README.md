# Cecko Ref

The reference solution of Cecko. 

### Merging changes from the Skeleton repository

In each local copy, create the skeleton_master branch to mirror the state of the skeleton:

```bash
git remote add upstream git@gitlab.mff.cuni.cz:teaching/nswi098/cecko/skeleton.git
git fetch upstream master:skeleton_master
```

Repeat the fetch every time Skeleton repository is updated, 
then merge the changes to the local master:

```bash
git merge skeleton_master
```

See also [Creating your Cecko repository](doc/Repository.md)

### Publishing skeleton updates back to the Skeleton repository

**Put all updates to the skeletal parts to separated commits.** 
These commits shall not contain any changes to the reference solution.

Switch to the skeleton_master branch, cherry-pick the skeleton updates, 
switch back to the master and merge them back to the local master:

```bash
git checkout skeleton_master
git cherry-pick <commit>...
git checkout master
git merge skeleton_master
```

Check the results in the git Log.
Push the skeleton_master changes to the Skeleton repository:

```bash
git push upstream skeleton_master:master
```

## Getting Started

* [Prerequisities](doc/Prerequisities.md)
* [Creating your Cecko repository](doc/Repository.md)
* [Building Cecko](doc/Building.md)
* [Running and debugging Cecko](doc/Runnning.md)

