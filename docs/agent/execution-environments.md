# 执行环境详细规则

> 仅在需要执行命令、排查环境混用或设计跨环境流程时读取。

# SS928 execution environments

This repository is edited from a Windows-native Codex environment. The integrated shell is PowerShell.

## Mandatory execution target

Before every command, identify exactly one target and state it in the response:

- `LOCAL-WIN`: the Windows host. Use PowerShell and Windows paths such as `C:\Users\ASUS\Desktop\ss928`.
- `BOARD-LINUX`: the remote SS928 board. Run commands only through SSH or SFTP; use Linux paths and Linux commands only after connecting to the board.
- `WSL-BUILD`: a temporary Linux build environment, used only when the user explicitly requests a WSL/Linux cross-compilation task.

Do not infer a target from the command syntax. If the target is not clear, ask before running a command.

## Local Windows rules (`LOCAL-WIN`)

- The default environment is Windows native, not WSL.
- Use PowerShell commands and Windows paths.
- Do not run bare Linux commands such as `ls`, `chmod`, `sudo`, `systemctl`, or `./script.sh` as if they were local Windows commands.
- Local validation commands may use installed Windows tools such as `node`, `python`, `git`, and PowerShell.

## SS928 board rules (`BOARD-LINUX`)

- Treat the board as a separate remote Linux machine, never as the local terminal environment.
- State `Execution target: BOARD-LINUX (SSH)` before a board operation.
- Run board commands through SSH, for example: `ssh <board-host> "uname -a"`.
- Use SFTP/SCP for file transfer. Do not use Windows copy commands to claim a file was uploaded to the board.
- Prefer read-only probes first. Do not change network, SSH, startup scripts, services, pinmux, or hardware state unless the user explicitly requests it.
- When an SS928 board-debug skill applies, read and follow it before connecting or running remote commands.

## WSL exception (`WSL-BUILD`)

- Do not switch the task's default environment or integrated terminal to WSL.
- Use WSL only for an explicitly requested Linux build or cross-compilation step.
- Label every such command `Execution target: WSL-BUILD` and invoke it explicitly from PowerShell (for example, `wsl bash -lc "..."`).
- Never run board operations inside WSL merely because both use Linux commands.

## Command-output safety

- Never mix commands for different targets in one unlabelled command block.
- Keep file paths native to the target: `C:\...` for `LOCAL-WIN`, `/root/...` and `/dev/...` for `BOARD-LINUX`, and `/home/...` for `WSL-BUILD`.
- After a command fails, first verify that it was run in the correct target before changing the command or diagnosing the software.

---
