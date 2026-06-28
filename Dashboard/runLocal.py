# -*- encoding: utf-8 -*-
import os
from   flask_migrate import Migrate
from   flask_minify  import Minify
from   sys import exit

from apps.config import config_dict
from apps import create_app, db

# WARNING: Don't run with debug turned on in production!
DEBUG = 'True'

# The configuration
get_config_mode = 'Debug' if DEBUG else 'Production'

try:
    app_config = config_dict[get_config_mode.capitalize()]

except KeyError:
    exit('Error: Invalid <config_mode>. Expected values [Debug, Production] ')

app = create_app(app_config)
Migrate(app, db)



if not DEBUG:
    Minify(app=app, html=True, js=False, cssless=False)
    


if __name__ == "__main__":
    app.run(host='0.0.0.0', port=3000)
