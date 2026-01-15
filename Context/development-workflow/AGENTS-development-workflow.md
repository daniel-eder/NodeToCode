# NodeToCode Development Workflow

Git submodule setup and IDE configuration for multi-version UE plugin development.

## Git Submodule Development Workflow

### Setting Up Multi-Version UE Plugin Development with Git Submodules

This workflow allows you to develop the NodeToCode plugin in a single location while testing it across multiple Unreal Engine project versions using git submodules.

### Initial Setup

1. **Initialize Git in UE Test Projects**
   ```bash
   cd "/Volumes/NVME 1/Projects/NodeToCode/NodeToCodeHostProjects/NodeToCodeHost_5_4"
   git init
   git add .
   git commit -m "Initial project setup"
   ```

2. **Add Plugin as Git Submodule**
   ```bash
   # Using relative path (recommended for same filesystem)
   git submodule add ../../NodeToCode Plugins/NodeToCode

   # Or using absolute path
   git submodule add "/Volumes/NVME 1/Projects/NodeToCode/NodeToCode" Plugins/NodeToCode
   ```

3. **Handle File Transport Errors**
   If you encounter "fatal: transport 'file' not allowed":
   ```bash
   # Allow file protocol globally
   git config --global protocol.file.allow always

   # Or one-time allow
   git -c protocol.file.allow=always submodule add "/path/to/plugin" Plugins/NodeToCode
   ```

4. **Commit Submodule Addition**
   ```bash
   git add .gitmodules Plugins/NodeToCode
   git commit -m "Add NodeToCode plugin as submodule"
   ```

### Development Workflow

1. **Update Submodule to Latest Changes**
   ```bash
   # Method 1: Update from parent project
   cd "/Volumes/NVME 1/Projects/NodeToCode/NodeToCodeHostProjects/NodeToCodeHost_5_4"
   git submodule update --remote --merge Plugins/NodeToCode

   # Method 2: Pull directly in submodule
   cd Plugins/NodeToCode
   git pull origin main  # or your working branch
   ```

2. **Make Changes in Submodule**
   ```bash
   cd Plugins/NodeToCode
   # Edit files, then commit
   git add .
   git commit -m "Your changes"
   git push origin your-branch
   ```

3. **Update Parent Project Reference**
   ```bash
   cd ../..  # Back to UE project root
   git add Plugins/NodeToCode
   git commit -m "Update NodeToCode plugin to latest"
   ```

4. **Track Specific Branch**
   ```bash
   git config -f .gitmodules submodule.Plugins/NodeToCode.branch your-branch-name
   git submodule update --remote
   ```

### Working with Local Branches

When developing on a local branch that hasn't been pushed to GitHub:

1. **Use local file paths** instead of GitHub URLs
2. **Changes are immediately reflected** in all projects using the submodule
3. **No need to push** to remote before testing

### Advantages of Submodule Approach

- Single plugin codebase for all UE versions
- Version control tracks specific plugin commits per project
- Can use different branches/versions per UE version
- Better for team collaboration
- Works seamlessly across machines

### Rider Configuration with Submodules

1. **Open the UE test project** (not just the plugin)
2. Rider will recognize the submodule structure
3. **Enable VCS integration** for submodules:
   - `Settings` → `Version Control` → `Git`
   - Check "Update submodules"
4. Changes made in Rider are reflected in the submodule's git repository
