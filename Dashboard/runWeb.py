# -*- encoding: utf-8 -*-
import os
from sys import exit
from flask import request, abort

# Gevent monkey-patching (must be done before importing other modules)
from gevent import monkey
monkey.patch_all()
import requests
import ssl
from flask_migrate import Migrate
from flask_minify import Minify
from gevent.pywsgi import WSGIServer
from apps.config import config_dict
from apps import create_app, db

# WARNING: Don't run with debug turned on in production!
DEBUG = 'False'
get_config_mode = 'Debug' if DEBUG else 'Production'

try:
    app_config = config_dict[get_config_mode.capitalize()]
except KeyError:
    exit('Error: Invalid <config_mode>. Expected values [Debug, Production]')

app = create_app(app_config)

Migrate(app, db)

if __name__ == "__main__":
    # Create a secure SSL context
    # SSL Certificate Configuration
    cert_file = "certificate.crt"
    key_file = "private.key"

    if not os.path.exists(cert_file) or not os.path.exists(key_file):
        print("Certificate or key not found. Generating self-signed certificate...")
        try:
            from werkzeug.serving import make_ssl_devcert
            # Generates certificate.crt and certificate.key
            make_ssl_devcert('certificate', host='localhost')
            if os.path.exists("certificate.key"):
                if os.path.exists(key_file):
                    os.remove(key_file)
                os.rename("certificate.key", key_file)
            print("Certificate generated successfully.")
        except ImportError:
            print("Error: 'cryptography' library is required to generate SSL certificates.")
            print("Please run: pip install cryptography")
            exit(1)
        except Exception as e:
            print(f"Error generating certificate: {e}")
            exit(1)

    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    try:
        context.load_cert_chain(certfile=cert_file, keyfile=key_file)
    except Exception as e:
        app.logger.error(f"Error loading SSL certificate: {e}")
        exit(1)

    # Create and start the Gevent WSGI server with SSL
    http_server = WSGIServer(('0.0.0.0', 443), app, ssl_context=context)
    app.logger.info("Starting secure WSGI server on port 443...")
    http_server.serve_forever()
