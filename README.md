# USE git pull FREQUENTLY!!!

# README.md is a MarkDown file. Look up markdown syntax if you're interested in changing text size and stuff
# Instructions
CLONE the repo first like in class
Workflow:
Each person work on own branch.

To create branch, 
    `git checkout -b branch_name`

After coding, push it.
    `git add .`
    `git commit -m "Add keyboard input"`
    `git push -u origin branch_name`
To return  to main branch,
    `git checkout -b main`
Now wait on pull request merge. See "What I Learned" below for more info.
Make sure your local code is updated with the following:
    `git pull`

# Git push shortcut
After you've done git push -u... can now just do git push and git pull without the retyping that branch name.
Note: For a future, new branch, must redo `git push -u origin new_branch`, and only then can you do `git push` and `git pull` without the branch name.
Seems to me like this shortcut is useful when you are making small commits over time on the same branch.
If you finished writing new code on a branch, tested it, etc. and you don't plan on continuing that branch, `git push -u origin branch_name` is enough. Get back to main branch after.

# What I Learned
Seems like doing the branches and pushing them leads to the creation of a pull request. 
This means the new code from that branch is not merged into main yet, and it is awaiting approval (we both have permission, just do it on github). This is called a pull request.
Keep in mind your local copy of the code will not be updated with your branch unless you:
    - merge your branch immediately into `main` after pushing. 
    - Pros: fast, no waiting.
    - Cons: 
        - Removes safety net of review.
        - Easier to accidentally break `main`.
        - Could miss out on learning branching and PR workflows. 
I know this slows down the whole process, but I think it would be a good learning experience, as that is how it works in the industry.

# Compile command
gcc main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o main && ./main

# CURRENT PROGRESS NOTES
- Made automove functionality

# FUTURE TODO's (No particular order)
- Enemies
- Shoot mechanic
- Collision 