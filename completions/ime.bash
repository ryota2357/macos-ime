_ime() {
  local cur
  cur="${COMP_WORDS[COMP_CWORD]}"

  if [[ $COMP_CWORD -eq 1 ]]; then
    COMPREPLY=($(compgen -W "get set list help" -- "$cur"))
    return
  fi

  if [[ "${COMP_WORDS[1]}" == "set" && $COMP_CWORD -eq 2 ]]; then
    local IFS=$'\n'
    COMPREPLY=($(compgen -W "$(ime list 2>/dev/null)" -- "$cur"))
  fi
}
complete -F _ime ime
