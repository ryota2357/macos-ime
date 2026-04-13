complete --command ime --no-files

complete --command ime --condition __fish_use_subcommand --arguments get  --description 'Print the current keyboard input source ID'
complete --command ime --condition __fish_use_subcommand --arguments set  --description 'Switch to the specified input source ID'
complete --command ime --condition __fish_use_subcommand --arguments list --description 'List enabled keyboard input source IDs'
complete --command ime --condition __fish_use_subcommand --arguments help --description 'Print help'

complete --command ime --condition '__fish_seen_subcommand_from set; and __fish_is_nth_token 2' --arguments '(ime list)'
