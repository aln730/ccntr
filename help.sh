#! /usr/bin/bash

# DISPLAY ROUTINES {{{
if tput colors >/dev/null 2>&1; then
    reset="$(tput sgr0)"

    bold="$(tput bold)"
    # tput does not have a default capability to turn bold off, but most often
    # it is 21.
    normal="\e[21m"
    underline="$(tput smul)"
    nounderline="$(tput rmul)"

    red="$(tput setaf 1)"
fi
# DISPLAY ROUTINES }}}

echo -e "Environment-related targets:"
echo
echo -e "\t$bold${red}setup$reset: setup the environment"
echo -e "\t\tsee ${underline}setup.sh$nounderline"
echo
echo -e "\t$bold${red}undosetup$reset: undo all steps done by ${bold}setup$normal"
echo -e "\t\trequired to run ${bold}setup$normal again"
echo
echo -e "\t$bold${red}reset$reset: reset the environment to a clean setup"
echo -e "\t\trun as a dependency of all test targets"
echo
echo -e "Test targets (in order of completion):"
echo
echo -e "\t$bold${red}run-namespaces$reset: test namespaces (except ${underline}mnt$nounderline namespace)"
echo
echo -e "\t$bold${red}run-namespace-mount$reset: test virtual filesystem in a ${underline}mnt$nounderline namespace"
echo
echo -e "\t$bold${red}run-cgroups$reset: test cgroups"
echo
echo -e "\t$bold${red}run-overlayfs$reset: test overlay virtual filesystem"
echo
echo -e "\t$bold${red}run-networking$reset: test base networking"
echo
echo -e "\t$bold${red}run-networking-webserver$reset: test complete networking with a web server"
echo
echo -e "Source code-related targets:"
echo
echo -e "\t$bold${red}container$reset (also $bold${red}all$reset, also the default target): compile the container engine"
echo
echo -e "\t$bold${red}tags$reset: build C tags"
echo
echo -e "\t$bold${red}clean$reset: clean compiled artifacts"
echo
echo -e "\t$bold${red}cleantags$reset: clean C tags"
echo
echo -e "\t$bold${red}mrproper$reset: $bold${red}clean$reset + $bold${red}cleantags$reset"
echo
