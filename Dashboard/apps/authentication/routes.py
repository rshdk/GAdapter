# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""

from functools import wraps
from flask import abort, render_template, redirect, request, url_for
from flask_login import (
    current_user,
    login_user,
    logout_user
)
import random
from flask_dance.contrib.github import github

from apps import db, login_manager
from apps.authentication import blueprint
from apps.authentication.forms import LoginForm, CreateAccountForm, UpdateAccountForm
from apps.authentication.models import Users

from apps.authentication.util import hash_pass, verify_pass

@blueprint.route('/')
def route_default():
    if current_user.is_authenticated:
        return redirect(url_for('home_blueprint.dashboard'))
    return redirect(url_for('authentication_blueprint.login'))



import random

@blueprint.route('/login', methods=['GET', 'POST'])
def login():
    login_form = LoginForm(request.form)

    if 'login' in request.form:
        # read form data
        username = request.form['username']
        password = request.form['password']

        # Locate user
        user = Users.query.filter_by(username=username).first()

        # Check the password
        if user and verify_pass(password, user.password):
            login_user(user)
            return render_template('home/dashboard.html',
                                   segment='dashboard',
                                   user_id=current_user.id,
                                   random=random)  # pass random to template

        # Something (user or pass) is not ok
        return render_template('accounts/login.html',
                               msg='Wrong user or password',
                               form=login_form)

    if not current_user.is_authenticated:
        return render_template('accounts/login.html',
                               form=login_form)

    # Redirect to the correct endpoint
    return redirect(url_for('home_blueprint.dashboard'))

from functools import wraps
from flask import abort, render_template, redirect, request, url_for
from flask_login import (
    current_user,
    login_user,
    logout_user
)
import random
from flask_dance.contrib.github import github

from apps import db, login_manager
from apps.authentication import blueprint
from apps.authentication.forms import LoginForm, CreateAccountForm, UpdateAccountForm
from apps.authentication.models import Users

from apps.authentication.util import hash_pass, verify_pass

import logging
logger = logging.getLogger(__name__)

def admin_required(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if not current_user.is_authenticated or not current_user.is_admin:
            abort(403)
        return f(*args, **kwargs)
    return decorated_function


@blueprint.route('/user_management', methods=['GET', 'POST'])
@admin_required
def user_management():
    create_account_form = CreateAccountForm(request.form)
    update_account_form = UpdateAccountForm(request.form)

    if 'create_user' in request.form:
        # create user
        user = Users(**request.form)
        db.session.add(user)
        db.session.commit()

    elif 'update_user' in request.form:
        user_id = request.form['user_id']
        user = Users.query.get(user_id)
        if user:
            user.username = request.form['username']
            user.email = request.form['email']
            if request.form.get('password'):  # only update if not empty
                user.password = hash_pass(request.form['password'])
            db.session.commit()
    elif 'delete_user' in request.form:
        user_id = request.form['user_id']
        user = Users.query.get(user_id)
        if user:
            db.session.delete(user)
            db.session.commit()

    users = Users.query.all()
    return render_template(
        'home/user_management.html',
        users=users,
        create_form=create_account_form,
        update_form=update_account_form,
        segment='user_management'
    )




@blueprint.route('/register', methods=['GET', 'POST'])
def register():
    create_account_form = CreateAccountForm(request.form)
    if 'register' in request.form:

        username = request.form['username']
        email = request.form['email']

        # Check usename exists
        user = Users.query.filter_by(username=username).first()
        if user:
            return render_template('accounts/register.html',
                                   msg='Username already registered',
                                   success=False,
                                   form=create_account_form)

        # Check email exists
        user = Users.query.filter_by(email=email).first()
        if user:
            return render_template('accounts/register.html',
                                   msg='Email already registered',
                                   success=False,
                                   form=create_account_form)

        # else we can create the user
        user = Users(**request.form)
        db.session.add(user)
        db.session.commit()

        # Delete user from session
        logout_user()
        
        return render_template('accounts/register.html',
                               msg='Account created successfully.',
                               success=True,
                               form=create_account_form)

    else:
        return render_template('accounts/register.html', form=create_account_form)


@blueprint.route('/logout')
def logout():
    logout_user()
    return redirect(url_for('authentication_blueprint.login'))

@blueprint.route('/make-admin/<int:user_id>', methods=['GET', 'POST'])
@admin_required
def make_admin(user_id):
    logger.info(f"--- make_admin START ---")
    logger.info(f"Attempting to make user with id {user_id} an admin.")
    user = Users.query.get(user_id)
    if user:
        logger.info(f"Found user: {user.username}")
        user.is_admin = True
        try:
            db.session.commit()
            logger.info(f"User {user.username} has been made an admin.")
        except Exception as e:
            logger.error(f"Error committing to database: {e}")
            db.session.rollback()
    else:
        logger.warning(f"User with id {user_id} not found.")
    logger.info(f"--- make_admin END ---")
    return redirect(url_for('authentication_blueprint.user_management'))

# Errors

@login_manager.unauthorized_handler
def unauthorized_handler():
    return render_template('home/page-403.html'), 403


@blueprint.errorhandler(403)
def access_forbidden(error):
    return render_template('home/page-403.html'), 403


@blueprint.errorhandler(404)
def not_found_error(error):
    return render_template('home/page-404.html'), 404


@blueprint.errorhandler(500)
def internal_error(error):
    return render_template('home/page-500.html'), 500
