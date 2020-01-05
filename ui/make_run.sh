#!/bin/bash
echo "#!/bin/bash" > run.sh

prefix="$1"

if [[ -n "${prefix}" ]]; then
  echo "env QT_QUICK_CONTROLS_CONF='${prefix}/qtquickcontrols2.conf' '$(pwd)/ChaosKit'" >> run.sh
else
  echo "'$(pwd)/ChaosKit'" >> run.sh
fi
chmod +x run.sh
