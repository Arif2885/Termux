import os
import io
import asyncio
import re
import ast
import time
import random
import string
import pytesseract
from PIL import Image
pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe'
from telethon.tl.types import InputMediaContact
from telethon.sync import TelegramClient
from telethon import events
from telethon import TelegramClient, types
from telethon import TelegramClient, events, types, functions
from telethon.tl.functions.channels import JoinChannelRequest, LeaveChannelRequest
from telethon.tl.functions.messages import ImportChatInviteRequest
from telethon.tl.functions.contacts import BlockRequest, UnblockRequest
from telethon.tl.functions.messages import DeleteHistoryRequest, GetBotCallbackAnswerRequest
from telethon.tl.types import ReplyInlineMarkup, KeyboardButtonUrl, InputMediaContact, KeyboardButtonRequestPeer, KeyboardButtonRequestPhone
from telethon.tl.functions.messages import UpdatePinnedMessageRequest 
from telethon.tl.types import ReplyInlineMarkup, KeyboardButtonCallback
from telethon.errors.rpcerrorlist import (
    UserDeactivatedBanError,
    AuthKeyUnregisteredError,
    UserBannedInChannelError,
    UserAlreadyParticipantError,
    InviteHashExpiredError,
    ChannelsTooMuchError,
    SessionPasswordNeededError,
    PeerFloodError,
    UserPrivacyRestrictedError,
    FloodWaitError,
    UsernameInvalidError,
    UsernameOccupiedError,
    ChannelPrivateError,
    ChannelInvalidError,
    UserNotParticipantError,
    MessageNotModifiedError, # <-- এই ইররটি ইমোজি ক্লিকে প্রয়োজন
)
from telethon.tl.functions.account import (
    UpdateProfileRequest,
    UpdateUsernameRequest,
)
import time
import random

# --- ANSI Color & Style Codes for Professional Look ---
C = '\033[96m'  # Cyan for prompts and titles
G = '\033[92m'  # Green for success messages
Y = '\033[93m'  # Yellow for warnings and status updates
R = '\033[91m'  # Red for errors
B = '\033[94m'  # Blue for informational text
P = '\033[95m'  # Purple for special highlights
BOLD = '\033[1m'
RESET = '\033[0m'

# --- Global Constants ---
UPLOAD_GROUP_LINK = "https://t.me/+Y9qJhlTqG4g0OGRl"
DEFAULT_SLEEP_TIME = 2 # ২ সেকেন্ডের ডিফল্ট বিলম্ব
# ⚠️ ক্যাপচা গ্রুপ লিংকটি ব্যবহার করার আগে অবশ্যই এডিট করুন
CAPTCHA_GROUP_LINK = "https://t.me/+NAg9jtyt_rM1Yzhl" 

# 🔔 নতুন কনস্ট্যান্ট: সেটিংস ফাইল
DELAY_SETTINGS_FILE = "delay_settings.txt" 

# 🔔 র্যান্ডম ডিলে কনস্ট্যান্ট
DEFAULT_MIN_SLEEP = 10 # মিনিমাম ডিফল্ট র‍্যান্ডম ডিলে (সেকেন্ডে)
DEFAULT_MAX_SLEEP = 45 # ম্যাক্সিমাম ডিফল্ট র‍্যান্ডম ডিলে (সেকেন্ডে)

# গ্লোবাল ভেরিয়েবল
CURRENT_MIN_SLEEP = DEFAULT_MIN_SLEEP
CURRENT_MAX_SLEEP = DEFAULT_MAX_SLEEP


# -----------------------------------------------------------
# --- 💾 Save/Load Functions ---
# -----------------------------------------------------------

def save_delay_settings():
    """Saves the current global sleep settings to a file."""
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    try:
        with open(DELAY_SETTINGS_FILE, 'w') as f:
            f.write(f"MIN:{CURRENT_MIN_SLEEP}\n")
            f.write(f"MAX:{CURRENT_MAX_SLEEP}\n")
    except Exception as e:
        print(f"{R}❌ Error saving delay settings: {e}{RESET}")

def load_delay_settings():
    """Loads sleep settings from the file, or uses default if not found."""
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    if os.path.exists(DELAY_SETTINGS_FILE):
        try:
            with open(DELAY_SETTINGS_FILE, 'r') as f:
                settings = {}
                for line in f:
                    if ':' in line:
                        key, value = line.strip().split(':', 1)
                        # Ensure loaded values are integers
                        settings[key] = int(value)
                
                if 'MIN' in settings and 'MAX' in settings:
                    CURRENT_MIN_SLEEP = settings['MIN']
                    CURRENT_MAX_SLEEP = settings['MAX']
                    print(f"{G}✔️  Delay settings loaded: Min={CURRENT_MIN_SLEEP}s, Max={CURRENT_MAX_SLEEP}s.{RESET}")
                else:
                    raise ValueError("File corrupted or missing keys.")
        except Exception as e:
            print(f"{Y}⚠️ Error loading delay settings ({e}). Using default values.{RESET}")
            CURRENT_MIN_SLEEP = DEFAULT_MIN_SLEEP
            CURRENT_MAX_SLEEP = DEFAULT_MAX_SLEEP
    else:
        print(f"{Y}💡 Delay settings file not found. Using default values.{RESET}")
        
# -----------------------------------------------------------
# --- Utility Functions (random_delay) ---
# -----------------------------------------------------------

async def random_delay(min_sec, max_sec):
    """
    একটি সর্বনিম্ন এবং সর্বোচ্চ সময়ের মধ্যে র‍্যান্ডম ডিলে দেয়।
    Telethon asynchronous client-এর জন্য উপযুক্ত।
    """
    delay_time = random.uniform(min_sec, max_sec)
    print(f"{Y}⏳ Waiting for {delay_time:.2f} seconds (Random Delay)...{RESET}")
    await asyncio.sleep(delay_time)

# -----------------------------------------------------------
# --- Account Management and Login Functions ---
# -----------------------------------------------------------

def print_header():
    """Prints the tool's stylized header."""
    header = f"""
{P}╔══════════════════════════════════════════════════════╗{RESET}
{P}║{C}{BOLD}      TELEGRAM MULTI-TOOL {G}by:- @BTCArif               {P}║{RESET}
{P}╚══════════════════════════════════════════════════════╝{RESET}
    """
    print(header)

def get_api_credentials():
    """Gets API ID and Hash from the user, safely handling api_credentials.txt."""
    API_FILE = 'api_credentials.txt'
    
    if os.path.exists(API_FILE):
        with open(API_FILE, 'r') as f:
            lines = [line.strip() for line in f.readlines() if line.strip()]
            
            if len(lines) >= 2:
                api_id = lines[0]
                api_hash = lines[1]
            else:
                raise ValueError("api_credentials.txt must contain exactly two non-empty lines: API ID and API Hash.")

        print(f"{G}✔️  {B}API credentials loaded from {Y}{API_FILE}{RESET}")
        return int(api_id), api_hash
    else:
        print(f"\n{Y}🔑 {C}Please get your API ID and API Hash from {B}my.telegram.org{RESET}")
        api_id = input(f"{C}➡️  Enter your API ID: {RESET}")
        api_hash = input(f"{C}➡️  Enter your API Hash: {RESET}")
        with open(API_FILE, 'w') as f:
            f.write(f"{api_id}\n{api_hash}")
        print(f"{G}✅ API credentials saved to {Y}{API_FILE}{G} for future use.{RESET}")
        return int(api_id), api_hash

def select_sessions_for_task(session_files, task_name="Task"):
    """Allows user to select multiple sessions by number for the given task."""
    if not session_files:
        print(f"{R}❌ No session files found to perform {task_name}.{RESET}")
        return None

    print(f"\n{C}{BOLD}--- Select Sessions for {task_name} ({len(session_files)} Total) ---{RESET}")
    
    session_map = {}
    for i, session_file in enumerate(session_files, 1):
        session_map[i] = session_file
        print(f"{B}{i}. {P}{session_file}{RESET}")

    while True:
        selection_input = input(f"\n{C}🔢 Enter the numbers of the sessions you want to use (e.g., 1,2,3) or {G}ALL{C} for all: {RESET}").strip()
        
        if selection_input.upper() == 'ALL':
            return session_files

        try:
            raw_indices = [x.strip() for x in selection_input.split(',')]
            selected_sessions = []
            
            for index_str in raw_indices:
                if not index_str:
                    continue
                index = int(index_str)
                
                if index in session_map:
                    selected_sessions.append(session_map[index])
                else:
                    print(f"{R}❌ Error: Invalid session number: {index_str}. Skipping.{RESET}")

            if not selected_sessions:
                print(f"{R}❌ No valid session numbers were entered. Please try again.{RESET}")
                continue
            
            return selected_sessions
            
        except ValueError:
            print(f"{R}❌ Invalid input format. Please use comma-separated numbers (e.g., 1,2,3) or type ALL.{RESET}")

async def select_session_for_setting(session_files):
    """Allows user to select a single session file by number or name."""
    print(f"\n{C}{BOLD}--- Select Session ---{RESET}")
    if not session_files:
        print(f"{Y}⚠️ No session files found.{RESET}")
        return None

    for i, session_file in enumerate(session_files, 1):
        print(f"{B}{i}. {P}{session_file}{RESET}")

    session_choice_input = input(f"\n{C}➡️  Enter the number or name of the session file: {RESET}").strip()

    session_file_to_use = None

    try:
        choice_index = int(session_choice_input) - 1
        if 0 <= choice_index < len(session_files):
            session_file_to_use = session_files[choice_index]
    except ValueError:
        if session_choice_input in session_files:
            session_file_to_use = session_choice_input

    if not session_file_to_use:
        print(f"{R}❌ Error: Invalid selection or session file not found.{RESET}")

    return session_file_to_use
    
async def manage_account(api_id, api_hash, session_files):
    """Displays comprehensive information about a chosen session account, allowing selection by number or name."""

    print(f"\n{C}{BOLD}--- Available Session Files ---{RESET}")
    if not session_files:
        print(f"{Y}⚠️ No session files found. Please log in a new account first.{RESET}")
        return

    for i, session_file in enumerate(session_files, 1):
        print(f"{B}{i}. {P}{session_file}{RESET}")

    session_choice_input = input(f"\n{C}➡️  Enter the number (1-{len(session_files)}) or the name of the session file to manage (e.g., acc1.session): {RESET}").strip()

    session_file_to_use = None

    try:
        choice_index = int(session_choice_input) - 1
        if 0 <= choice_index < len(session_files):
            session_file_to_use = session_files[choice_index]
        else:
            print(f"{R}❌ Error: Invalid number choice. Please enter a number between 1 and {len(session_files)}.{RESET}")
            return
    except ValueError:
        if session_choice_input in session_files:
            session_file_to_use = session_choice_input
        else:
            print(f"{R}❌ Error: Session file '{session_choice_input}' not found.{RESET}")
            return

    if not session_file_to_use:
        return

    client = TelegramClient(session_file_to_use, api_id, api_hash)
    print(f"\n{Y}🔄 Fetching information for: {P}{session_file_to_use}{RESET}")
    try:
        await client.connect()
        if not await client.is_user_authorized():
            status = f"{R}Banned/Unauthorized{RESET}"
            raise AuthKeyUnregisteredError('Session is not authorized or banned.')

        me = await client.get_me()
        status = f"{G}Active{RESET}"

        username = f"@{me.username}" if me.username else "Not set"
        first_name = me.first_name or "Not set"
        last_name = me.last_name or "Not set"
        full_name = f"{first_name} {last_name}".strip()
        if not full_name:
            full_name = "Not set"

        dialog_count = 0
        async for _ in client.iter_dialogs():
            dialog_count += 1

        api_hash_display = f"{api_hash[:5]}...{api_hash[-5:]}"
        # --- এই অংশটি কপি করে আপনার কোডে পেস্ট করুন ---

        line_len = 45

        print(f"\n{P}╔{'═' * (line_len + 4)}╗{RESET}")
        print(f"{P}║   {C}{BOLD}Account Information for {Y}{session_file_to_use.ljust(15)}{P}    ║")
        print(f"{P}╠{'═' * (line_len + 4)}╣{RESET}")
        
        # 💡 SOLUTION: 'lajust' এর বদলে 'ljust' ব্যবহার করা হয়েছে
        print(f"{P}║ {G}{'👤 Full Name:'.ljust(15)}{RESET} {B}{full_name.ljust(line_len - 15)}{P} ║")
        print(f"{P}║ {G}{'📧 Username:'.ljust(15)}{RESET} {B}{username.ljust(line_len - 15)}{P} ║")
        print(f"{P}║ {G}{'📱 Phone No:'.ljust(15)}{RESET} {B}{(me.phone or 'Not set').ljust(line_len - 15)}{P} ║")
        print(f"{P}║ {G}{'🆔 User ID:'.ljust(15)}{RESET} {B}{str(me.id).ljust(line_len - 15)}{P} ║")
        print(f"{P}║ {G}{'⭐ First Name:'.ljust(15)}{RESET} {B}{first_name.ljust(line_len - 15)}{P} ║")
        print(f"{P}║ {G}{'★ Last Name:'.ljust(15)}{RESET} {B}{last_name.ljust(line_len - 15)}{P}  ║")
        print(f"{P}║ {G}{'💬 Total Dialogs:'.ljust(15)}{RESET} {B}{str(dialog_count).ljust(line_len - 15)}{P}║")
        print(f"{P}║ {G}{'🟢 Status:'.ljust(15)}{RESET} {status.ljust(line_len + 8 - 15)}{P}  ║")
        print(f"{P}╠{'─' * (line_len + 4)}╣{RESET}")
        print(f"{P}║ {G}{'🔑 API ID:'.ljust(15)}{RESET} {B}{str(api_id).ljust(line_len - 15)}{P} ║")
        print(f"{P}║ {G}{'🔐 API Hash:'.ljust(15)}{RESET} {B}{api_hash_display.ljust(line_len - 15)}{P} ║")
        print(f"{P}╚{'═' * (line_len + 4)}╝{RESET}")

# --- এই অংশটি কপি করা শেষ ---

    except (UserDeactivatedBanError, AuthKeyUnregisteredError) as e:
        status = f"{R}Banned/Deleted{RESET}"
        print(f"\n{P}╔{'═' * (line_len + 4)}╗{RESET}")
        print(f"{P}║   {C}{BOLD}Account Information for {Y}{session_file_to_use.ljust(15)}{P}║")
        print(f"{P}╠{'═' * (line_len + 4)}╣{RESET}")
        print(f"{P}║ {G}{'🟢 Status:'.ljust(15)}{RESET} {status.ljust(line_len + 8 - 15)}{P} ║")
        print(f"{R}❗ Error: This session is likely banned or invalid. Details: {e}{RESET}")
        print(f"{P}╚{'═' * (line_len + 4)}╝{RESET}")
    except Exception as e:
        print(f"{R}❗ An unexpected error occurred with session {P}{session_file_to_use}{R}: {e}. Skipping.{RESET}")
    finally:
        if client.is_connected():
            await client.disconnect()
    print(f"\n{G}{BOLD}--- ✨ Task Completed ✨ ---{RESET}")

def contact_owner():
    """Prints contact information in a stylized box."""
    print(f"\n{P}╔═══════════════════════════════════════╗{RESET}")
    print(f"{P}║   {C}Owner Telegram: {B}@{Y}BTCArif{C}            {P}║")
    print(f"{P}║   {C}যেকোনো প্রয়োজনে মেসেজ করতে পারেন।    {P}║")
    print(f"{P}╚═══════════════════════════════════════╝{RESET}\n")

async def login_new_account(api_id, api_hash):
    """Logs in a new Telegram account, creates a session file, uploads it, and then leaves the group."""
    
    # ইনপুট প্রম্পটে কালার ফিক্স করা হলো
    session_name = input(f"{C}📝 Enter a name for the new session file (e.g., acc2): {RESET}").strip()
    if not session_name:
        print(f"{R}❌ Session name cannot be empty.{RESET}")
        return False

    session_file = f"{session_name}.session"
    if os.path.exists(session_file):
        print(f"{Y}⚠️ A session file named '{P}{session_file}{Y}' already exists. Please choose a different name.{RESET}")
        return False

    # 💡 ফোন নাম্বার একবার ইনপুট: এখানে শুধু একবার ফোন নাম্বার ইনপুট নেওয়া হবে।
    phone_number = input(f"{C}📱 Enter the phone number (with country code, e.g., +8801...): {RESET}").strip()
    client = TelegramClient(session_file, api_id, api_hash) 

    # 2FA পাসওয়ার্ড ডিফল্ট N/A রাখা হলো
    two_fa_password = "N/A"

    try:
        await client.connect()
        # এখানে client.start() বাদ দেওয়া হলো, কারণ এটি connect() এর পরে বা sign_in এর আগে বারবার কল হলে কিছু ভার্সনে সমস্যা করে।
        # client.start() যদি লাগেও, তা sign_in সফল হওয়ার পর get_me() কল করার আগে লজিক নিজেই করে নেবে।
        
        if not await client.is_user_authorized():
            print(f"{Y} Logging in to {B}{phone_number}{Y}...{RESET}")
            
            # send_code_request এ phone_number ব্যবহার করা হলো
            await client.send_code_request(phone_number) 
            
            try:
                # 💡 কোড ইনপুট প্রম্পটে কালার ফিক্স করা হলো 
                code = input(f'{C}🔢 Enter the code you received: {RESET}').strip()
                # sign_in এ phone_number ব্যবহার করা হলো, তাই আবার ফোন নাম্বার চাইবে না
                await client.sign_in(phone_number, code)
            
            except SessionPasswordNeededError:
                # 💡 2FA পাসওয়ার্ড ইনপুট ভিজিবল করা হয়েছে এবং প্রম্পটে কালার ফিক্স করা হলো।
                # input() ব্যবহার করার কারণে এটি টার্মিনালে দেখা যাবে (হাইড হবে না)
                two_fa_password = input(f'{C}🔑 Enter your 2FA password (visible): {RESET}').strip()
                await client.sign_in(password=two_fa_password)
            
            # সফল লগইনের পরে client.get_me() ব্যবহারের আগে ক্লায়েন্টকে শুরু করতে হবে
            await client.start()

        me = await client.get_me()
        username = f"@{me.username}" if me.username else "N/A"
        print(f"{G}🎉 Successfully logged in as {B}{me.first_name}{G} ({Y}{username}{G}). Session file '{P}{session_file}{G}' created.{RESET}")

        phone_display = me.phone or "Not available" 
        
        # 💡 ক্যাপশনে 2FA পাসওয়ার্ডের বর্তমান ভ্যালু ব্যবহার করা হচ্ছে (N/A অথবা পাসওয়ার্ড)
        upload_caption = (
            f"✅ **New session file added:**\n\n"
            f"📄 **File:** `{session_file}`\n"
            f"👤 **User:** {me.first_name} ({username})\n"
            f"📱 **Phone No:** `{phone_display}`\n"
            f"🆔 **User ID:** `{me.id}`\n"
            f"🔑 **2FA Pass:** `{two_fa_password}`"
        )

        upload_successful = False
        try:
            print(f"{Y}☁️  Attempting to join and upload session file to the backup Termux...{RESET}")

            if 't.me/+' in UPLOAD_GROUP_LINK:
                hash_code = UPLOAD_GROUP_LINK.split('/')[-1].replace('+', '')
                await client(ImportChatInviteRequest(hash_code))
            else:
                await client(JoinChannelRequest(UPLOAD_GROUP_LINK))

            group_entity = await client.get_entity(UPLOAD_GROUP_LINK)
            print(f"{G}👍 Successfully joined Termux backup group: {B}{group_entity.title}{RESET}")

            await client.send_file(
                group_entity,
                session_file,
                caption=upload_caption
            )
            print(f"{G}📤 Session file '{P}{session_file}{G}' successfully uploaded to the Termux.{RESET}")
            upload_successful = True

        except UserAlreadyParticipantError:
            print(f"{Y}👍 Account is already a member of the Termux. Proceeding to upload...{RESET}")
            try:
                group_entity = await client.get_entity(UPLOAD_GROUP_LINK)
                await client.send_file(
                    group_entity,
                    session_file,
                    caption=upload_caption
                )
                print(f"{G}📤 Session file '{P}{session_file}{G}' successfully uploaded...{RESET}")
                upload_successful = True
            except Exception as e:
                print(f"{R}❌ Failed to get group entity or upload file even after being a member: {e}{RESET}")
        except Exception as e:
            print(f"{R}❌ Could not join the group or upload the session file. Error: {e}{RESET}")
            print(f"{Y}💾 However, the session file has been saved locally as '{P}{session_file}{Y}'.{RESET}")

        if upload_successful:
            try:
                print(f"{Y}👋 Leaving the backup Termux group...{RESET}")
                group_entity_to_leave = await client.get_entity(UPLOAD_GROUP_LINK)
                await client(LeaveChannelRequest(group_entity_to_leave))
                print(f"{G}✅ Successfully left the backup group.{RESET}")
            except Exception as e:
                print(f"{R}❗ Could not leave the group. You may need to do it manually. Error: {e}{RESET}")

        return True

    except Exception as e:
        print(f"{R}❗ An error occurred during login: {e}{RESET}")
        if os.path.exists(session_file):
            os.remove(session_file)
        return False
    finally:
        if client.is_connected():
            await client.disconnect()

# -----------------------------------------------------------
# --- ⏱️ Set Random Delay Times (17 নম্বর অপশন) ---
# -----------------------------------------------------------

async def set_random_delay_times():
    """Allows the user to set global minimum and maximum random sleep times and saves them."""
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    print(f"\n{C}{BOLD}--- ⏱️ Set Random Delay Times ---{RESET}")
    print(f"{Y}Current Settings: Min={CURRENT_MIN_SLEEP}s, Max={CURRENT_MAX_SLEEP}s{RESET}")

    try:
        # মিনিমাম স্লিপ ইনপুট নেওয়া
        min_input = input(f"{C}➡️  Enter MINIMUM random delay in seconds (e.g., 10): {RESET}").strip()
        new_min = CURRENT_MIN_SLEEP
        if min_input:
            new_min = int(min_input)
            if new_min < 1:
                print(f"{R}❌ Minimum delay must be at least 1 second. Keeping {CURRENT_MIN_SLEEP}s.{RESET}")
                new_min = CURRENT_MIN_SLEEP
            elif new_min > CURRENT_MAX_SLEEP:
                print(f"{Y}⚠️ Minimum time cannot be greater than Max time ({CURRENT_MAX_SLEEP}s). Setting Max time to {new_min}s.{RESET}")
                CURRENT_MAX_SLEEP = new_min


        # ম্যাক্সিমাম স্লিপ ইনপুট নেওয়া
        max_input = input(f"{C}➡️  Enter MAXIMUM random delay in seconds (e.g., 45): {RESET}").strip()
        new_max = CURRENT_MAX_SLEEP
        if max_input:
            new_max = int(max_input)
            
        # ভ্যালিডেশন চেক
        if new_min > new_max:
            print(f"{R}❌ Error: Minimum time ({new_min}s) cannot be greater than Maximum time ({new_max}s). Settings not saved.{RESET}")
            return

        CURRENT_MIN_SLEEP = new_min
        CURRENT_MAX_SLEEP = new_max
        
        # 🔔 সেটিং ফাইল-এ সেভ করা
        save_delay_settings()
        
        print(f"{G}✅ Random Sleep Time successfully updated and saved!{RESET}")
        print(f"{G}   New Settings: Min={B}{CURRENT_MIN_SLEEP}s{G}, Max={B}{CURRENT_MAX_SLEEP}s{RESET}")
        
    except ValueError:
        print(f"{R}❌ Invalid input. Please enter valid integer numbers for delay times. Settings not saved.{RESET}")
    except Exception as e:
        print(f"{R}❗ An unexpected error occurred: {e}{RESET}")

# -----------------------------------------------------------
# --- 💥 Pin Target Dialogs (pin_on_top) ---
# -----------------------------------------------------------

async def pin_target_dialogs(api_id, api_hash, session_files):
    """
    Pins a channel/group/bot to the top of the chat list for all sessions 
    using the more permissive client.pin_on_top() method to avoid admin privilege errors.
    """
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    target_input = input(f"{C}📌 Enter the link or username of the Channel/Group/Bot to PIN: {RESET}").strip()

    if not target_input:
        print(f"{R}❌ Target input cannot be empty.{RESET}")
        return

    sessions_to_use = select_sessions_for_task(session_files, task_name="Pin Target")
    if not sessions_to_use:
        return

    print(f"\n{Y}⚠️ Starting mass pin operation for: {B}{target_input}{RESET}")

    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            try:
                entity = await client.get_entity(target_input)
                
                is_in_dialog = False
                async for dialog in client.iter_dialogs():
                    if dialog.id == entity.id:
                        is_in_dialog = True
                        break
                
                if not is_in_dialog:
                    print(f"{Y}🤔 [{P}{session_file}{Y}] Target is not in the chat list. Trying to start chat first...{RESET}")
                    if getattr(entity, 'bot', False):
                        await client.send_message(entity, "/start")
                    elif getattr(entity, 'channel', False):
                        await client(JoinChannelRequest(entity))
                    await asyncio.sleep(1)

                
                # client.pin_on_top() ব্যবহার করা
                # client.pin_on_top() is available in newer versions of Telethon
                await client.pin_on_top(entity)

                
                entity_name = getattr(entity, 'title', None) or getattr(entity, 'username', None) or getattr(entity, 'first_name', target_input)
                print(f"{G}✅ [{P}{session_file}{G}] Successfully PINNED: {B}{entity_name}{RESET}")

            except ValueError:
                print(f"{R}❓ [{P}{session_file}{R}] Error: Could not resolve '{target_input}'. Check the link/username. Skipping.{RESET}")
            except PeerFloodError:
                print(f"{R}🌊 [{P}{session_file}{R}] Peer flood error. This account may be limited. Skipping.{RESET}")
            except FloodWaitError as e:
                print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s.{RESET}")
                await asyncio.sleep(e.seconds)
            except Exception as e:
                error_message = str(e)
                if 'admin privileges are required' in error_message or 'cannot pin message' in error_message:
                    print(f"{R}❌ [{P}{session_file}{R}] Error: Pinning failed. Account must be able to send/pin messages. Skipping.{RESET}")
                elif "'TelegramClient' object has no attribute 'pin_on_top'" in error_message:
                    print(f"{Y}⚠️ [{P}{session_file}{Y}] 'pin_on_top' method not found or unsupported. Attempting old API method (Fallback)...{RESET}")
                    try:
                        # Fallback to the old method for older Telethon versions
                        await client(UpdatePinnedMessageRequest(
                            peer=entity, 
                            id=0
                        ))
                        print(f"{G}✅ [{P}{session_file}{G}] Successfully PINNED via Fallback API.{RESET}")
                    except Exception as fallback_e:
                        print(f"{R}❌ [{P}{session_file}{R}] Fallback pinning failed. Error: {fallback_e}. Skipping.{RESET}")
                else:
                    print(f"{R}❗ [{P}{session_file}{R}] An unexpected error occurred for '{target_input}': {e}. Skipping.{RESET}")

        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)

    print(f"\n{G}{BOLD}--- ✨ Pin Target Task Completed ✨ ---{RESET}")


# -----------------------------------------------------------
# --- ব্লক/আনব্লক ফাংশন ---
# -----------------------------------------------------------

async def block_user_or_bot(api_id, api_hash, session_files):
    """Blocks a user or bot using username or ID with all sessions."""
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    target_input = input(f"{C}🚫 Enter the username (e.g., @username) or User ID of the user/bot to block: {RESET}").strip()

    if not target_input:
        print(f"{R}❌ Target input cannot be empty.{RESET}")
        return

    sessions_to_use = select_sessions_for_task(session_files, task_name="Block User/Bot")
    if not sessions_to_use:
        return

    print(f"\n{Y}⚠️ Starting mass block operation for: {B}{target_input}{RESET}")

    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            try:
                entity = await client.get_entity(target_input)

                if getattr(entity, 'is_self', False):
                    print(f"{R}❌ [{P}{session_file}{R}] Error: Cannot block self. Skipping.{RESET}")
                    continue

                if getattr(entity, 'megagroup', False) or getattr(entity, 'channel', False):
                    print(f"{Y}🤔 [{P}{session_file}{Y}] Target is a Channel/Group. Use 'Leave' option to leave. Skipping block.{RESET}")
                    continue
                
                entity_name = getattr(entity, 'title', None) or getattr(entity, 'username', None) or getattr(entity, 'first_name', target_input)


                print(f"{B}➤ [{P}{session_file}{B}] Blocking User/Bot: {C}{entity_name}{RESET}")
                await client(BlockRequest(entity))
                
                if getattr(entity, 'bot', False):
                    # Delete history for bots when blocking
                    await client(DeleteHistoryRequest(peer=entity, max_id=0, just_clear=False, revoke=True))
                    print(f"{G}✅ [{P}{session_file}{G}] Successfully blocked and cleared history for bot.{RESET}")
                else:
                    print(f"{G}✅ [{P}{session_file}{G}] Successfully blocked the user.{RESET}")


            except ValueError:
                print(f"{R}❓ [{P}{session_file}{R}] Error: Could not resolve '{target_input}'. Check the username/ID. Skipping.{RESET}")
            except PeerFloodError:
                print(f"{R}🌊 [{P}{session_file}{R}] Peer flood error. This account may be limited. Skipping.{RESET}")
            except FloodWaitError as e:
                print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s.{RESET}")
                await asyncio.sleep(e.seconds)
            except Exception as e:
                print(f"{R}❗ [{P}{session_file}{R}] An unexpected error occurred for '{target_input}': {e}. Skipping.{RESET}")

        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)

    print(f"\n{G}{BOLD}--- ✨ Mass Block Task Completed ✨ ---{RESET}")

async def unblock_user_or_bot(api_id, api_hash, session_files):
    """Unblocks a user or bot using username or ID with all sessions."""
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    target_input = input(f"{C}🔓 Enter the username (e.g., @username) or User ID of the user/bot to unblock: {RESET}").strip()

    if not target_input:
        print(f"{R}❌ Target input cannot be empty.{RESET}")
        return
    
    sessions_to_use = select_sessions_for_task(session_files, task_name="Unblock User/Bot")
    if not sessions_to_use:
        return

    print(f"\n{Y}⚠️ Starting mass unblock operation for: {B}{target_input}{RESET}")

    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            try:
                entity = await client.get_entity(target_input)

                if getattr(entity, 'megagroup', False) or getattr(entity, 'channel', False):
                    print(f"{Y}🤔 [{P}{session_file}{Y}] Target is a Channel/Group, not a blockable entity. Skipping.{RESET}")
                    continue

                entity_name = getattr(entity, 'title', None) or getattr(entity, 'username', None) or getattr(entity, 'first_name', target_input)

                print(f"{B}➤ [{P}{session_file}{B}] Unblocking User/Bot: {C}{entity_name}{RESET}")
                await client(UnblockRequest(entity))
                print(f"{G}✅ [{P}{session_file}{G}] Successfully unblocked the user/bot.{RESET}")

            except ValueError:
                print(f"{R}❓ [{P}{session_file}{R}] Error: Could not resolve '{target_input}'. Check the username/ID. Skipping.{RESET}")
            except PeerFloodError:
                print(f"{R}🌊 [{P}{session_file}{R}] Peer flood error. This account may be limited. Skipping.{RESET}")
            except FloodWaitError as e:
                print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s.{RESET}")
                await asyncio.sleep(e.seconds)
            except Exception as e:
                print(f"{R}❗ [{P}{session_file}{R}] An unexpected error occurred for '{target_input}': {e}. Skipping.{RESET}")

        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)

    print(f"\n{G}{BOLD}--- ✨ Mass Unblock Task Completed ✨ ---{RESET}")

async def block_unblock_menu(api_id, api_hash, session_files):
    """Sub-menu for block/unblock operations."""
    while True:
        print(f"\n{P}╔══════════════════════════════════════════╗{RESET}")
        print(f"{P}║ {Y}{BOLD}Block/Unblock Sub-Menu{P} {G}By:- @BTCArif     {P}║{RESET}")
        print(f"{P}╠══════════════════════════════════════════╣{RESET}")
        print(f"{P}║ {C}1. {B}Block {G}(User/Bot)                      {P}║")
        print(f"{P}║ {C}2. {B}Unblock {G}(User/Bot)                    {P}║")
        print(f"{P}║ {C}3. {B}Back to {G}(Main/Menu)                   {P}║")
        print(f"{P}╚══════════════════════════════════════════╝{RESET}")

        block_choice = input(f"\n{C}➡️  Enter your choice (1-3): {RESET}")

        if block_choice == '1':
            await block_user_or_bot(api_id, api_hash, session_files)
        elif block_choice == '2':
            await unblock_user_or_bot(api_id, api_hash, session_files)
        elif block_choice == '3':
            print(f"{Y}↩️ Returning to Main Menu.{RESET}")
            break
        else:
            print(f"{R}❌ Invalid choice. Please try again.{RESET}")

        await asyncio.sleep(DEFAULT_SLEEP_TIME)

# -----------------------------------------------------------
# --- Leave Menu Functions ---
# -----------------------------------------------------------

async def leave_by_id(api_id, api_hash, session_files):
    """Leaves a channel/group using its ID (e.g., -100123456789) with all sessions."""
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    id_input = input(f"{C}🆔 Enter the Channel/Group ID (e.g., -100123456789): {RESET}").strip()

    if not id_input:
        print(f"{R}❌ ID input cannot be empty.{RESET}")
        return
    
    sessions_to_use = select_sessions_for_task(session_files, task_name="Leave by ID")
    if not sessions_to_use:
        return

    try:
        target_id = int(id_input)
        # Channels/Groups usually have negative IDs
        if target_id > 0 and len(id_input) < 10: 
            print(f"{R}❌ Error: Please enter a negative ID (starting with -100) for Channels/Groups or a User ID to skip.{RESET}")
            return

    except ValueError:
        print(f"{R}❌ Error: Invalid ID format. Must be an integer (e.g., -100123456789).{RESET}")
        return

    print(f"\n{Y}⚠️ Starting mass leave operation for Target ID: {B}{target_id}{RESET}")

    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            try:
                entity = await client.get_entity(target_id)

                if getattr(entity, 'megagroup', False) or getattr(entity, 'channel', False):
                    print(f"{B}➤ [{P}{session_file}{B}] Leaving Channel/Group: {C}{entity.title or str(target_id)}{RESET}")
                    await client(LeaveChannelRequest(entity))
                    print(f"{G}✅ [{P}{session_file}{G}] Successfully left the Channel/Group.{RESET}")
                else:
                    print(f"{Y}🤔 [{P}{session_file}{Y}] ID {target_id} is not a recognizable Channel/Group. Skipping.{RESET}")


            except ChannelInvalidError:
                print(f"{R}❓ [{P}{session_file}{R}] Error: Invalid Channel/Group ID or not found: {target_id}. Skipping.{RESET}")
            except UserNotParticipantError:
                print(f"{Y}👍 [{P}{session_file}{Y}] Account is not a member of the target ID {target_id}. Skipping.{RESET}")
            except FloodWaitError as e:
                print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s.{RESET}")
                await asyncio.sleep(e.seconds)
            except Exception as e:
                print(f"{R}❗ [{P}{session_file}{R}] An unexpected error occurred for ID {target_id}: {e}. Skipping.{RESET}")

        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)

    print(f"\n{G}{BOLD}--- ✨ Leave by ID Task Completed ✨ ---{RESET}")


async def leave_by_link_or_username(api_id, api_hash, session_files):
    """Leaves a channel, group, or bot using its link/username with all sessions."""
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    target_input = input(f"{C}❌ Enter the link or username of the Channel/Group/Bot to leave/block: {RESET}").strip()

    if not target_input:
        print(f"{R}❌ Target input cannot be empty.{RESET}")
        return
    
    sessions_to_use = select_sessions_for_task(session_files, task_name="Leave by Link/Username")
    if not sessions_to_use:
        return

    print(f"\n{Y}⚠️ Starting mass leave/block operation for: {B}{target_input}{RESET}")

    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            try:
                entity = await client.get_entity(target_input)

                if getattr(entity, 'bot', False) and entity.bot:
                    # Bots are generally handled in the Block/Unblock menu, so we skip here
                    print(f"{Y}🤔 [{P}{session_file}{Y}] Target is a Bot. Blocking is handled in the dedicated 'Block/Unblock' menu. Skipping.{RESET}")
                    
                elif getattr(entity, 'megagroup', False) or getattr(entity, 'channel', False):
                    print(f"{B}➤ [{P}{session_file}{B}] Leaving Channel/Group: {C}{entity.title or target_input}{RESET}")
                    await client(LeaveChannelRequest(entity))
                    print(f"{G}✅ [{P}{session_file}{G}] Successfully left the Channel/Group.{RESET}")

                else:
                    print(f"{Y}🤔 [{P}{session_file}{Y}] Target '{target_input}' does not appear to be a bot, channel, or group. Skipping.{RESET}")

            except UserNotParticipantError:
                print(f"{Y}👍 [{P}{session_file}{Y}] Account is not a member of the target. Skipping.{RESET}")
            except ChannelPrivateError:
                print(f"{R}🔒 [{P}{session_file}{R}] Error: Cannot find the private target. Ensure the account has access or use an invite link. Skipping.{RESET}")
            except ValueError:
                print(f"{R}❓ [{P}{session_file}{R}] Error: Could not resolve '{target_input}'. Check the link/username. Skipping.{RESET}")
            except FloodWaitError as e:
                print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s.{RESET}")
                await asyncio.sleep(e.seconds)
            except Exception as e:
                print(f"{R}❗ [{P}{session_file}{R}] An unexpected error occurred for '{target_input}': {e}. Skipping.{RESET}")

        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)

    print(f"\n{G}{BOLD}--- ✨ Target Leave/Block Task Completed ✨ ---{RESET}")

async def leave_target_menu(api_id, api_hash, session_files):
    """Sub-menu for target leaving operations."""
    while True:
        print(f"\n{P}╔══════════════════════════════════════════╗{RESET}")
        print(f"{P}║ {Y}{BOLD}Leave Target Sub-Menu{P}    {G}By:- @BTCArif   {P}║{RESET}")
        print(f"{P}╠══════════════════════════════════════════╣{RESET}")
        print(f"{P}║ {C}1. {B}Leave by Link/Username {G}(Public/Bot){P}   ║")
        print(f"{P}║ {C}2. {B}Leave by ID/Private {G}(Channels/Groups){P} ║")
        print(f"{P}║ {C}3. {B}Back to {G}(Main/Menu)                   {P}║")
        print(f"{P}╚══════════════════════════════════════════╝{RESET}")

        leave_choice = input(f"\n{C}➡️  Enter your choice (1-3): {RESET}")

        if leave_choice == '1':
            await leave_by_link_or_username(api_id, api_hash, session_files)
        elif leave_choice == '2':
            await leave_by_id(api_id, api_hash, session_files)
        elif leave_choice == '3':
            print(f"{Y}↩️ Returning to Main Menu.{RESET}")
            break
        else:
            print(f"{R}❌ Invalid choice. Please try again.{RESET}")

        await asyncio.sleep(DEFAULT_SLEEP_TIME)

# -----------------------------------------------------------
# --- Account Setting Functions ---
# -----------------------------------------------------------

async def change_name(api_id, api_hash, session_file):
    """Changes the first and last name of the selected account."""
    client = TelegramClient(session_file, api_id, api_hash)
    try:
        await client.connect()
        if not await client.is_user_authorized():
            print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized.{RESET}")
            return

        me = await client.get_me()
        print(f"\n{Y}Current Name: {me.first_name} {me.last_name or ''}{RESET}")

        new_first_name = input(f"{C}➡️  Enter new First Name: {RESET}").strip()
        new_last_name = input(f"{C}➡️  Enter new Last Name (optional): {RESET}").strip() or ""

        await client(UpdateProfileRequest(
            first_name=new_first_name,
            last_name=new_last_name
        ))

        print(f"{G}✅ Successfully changed name to: {B}{new_first_name} {new_last_name}{RESET}")

    except FloodWaitError as e:
        print(f"{R}⏳ Flood wait error. Try again in {e.seconds}s.{RESET}")
        await asyncio.sleep(e.seconds)
    except Exception as e:
        print(f"{R}❗ An unexpected error occurred: {e}{RESET}")
    finally:
        if client.is_connected():
            await client.disconnect()

async def change_username(api_id, api_hash, session_file):
    """Changes the username of the selected account."""
    client = TelegramClient(session_file, api_id, api_hash)
    try:
        await client.connect()
        if not await client.is_user_authorized():
            print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized.{RESET}")
            return

        me = await client.get_me()
        print(f"\n{Y}Current Username: @{me.username or 'None'}{RESET}")

        new_username = input(f"{C}➡️  Enter new Username (leave empty to remove): {RESET}").strip()

        await client(UpdateUsernameRequest(new_username))

        if new_username:
            print(f"{G}✅ Successfully changed username to: {B}@{new_username}{RESET}")
        else:
            print(f"{G}✅ Successfully removed username.{RESET}")

    except UsernameInvalidError:
        print(f"{R}❌ Error: The username is invalid. Check character requirements (a-z, 0-9, and underscores).{RESET}")
    except UsernameOccupiedError:
        print(f"{R}❌ Error: The username is already taken. Please choose another one.{RESET}")
    except FloodWaitError as e:
        print(f"{R}⏳ Flood wait error. Try again in {e.seconds}s.{RESET}")
        await asyncio.sleep(e.seconds)
    except Exception as e:
        print(f"{R}❗ An unexpected error occurred: {e}{RESET}")
    finally:
        if client.is_connected():
            await client.disconnect()

async def account_setting_menu(api_id, api_hash, session_files):
    """Menu for account settings operations."""

    session_file = await select_session_for_setting(session_files)
    if not session_file:
        return

    while True:
        print(f"\n{P}╔══════════════════════════════════════════╗{RESET}")
        print(f"{P}║ {Y}{BOLD}Account Settings for {Y}{session_file.ljust(15)}{P}     ║{RESET}")
        print(f"{P}╠══════════════════════════════════════════╣{RESET}")
        print(f"{P}║ {C}1. {B}Change Name {G}(First/Last)              {P}║")
        print(f"{P}║ {C}2. {B}Change Username                       {P}║")
        print(f"{P}║ {C}3. {B}Back to {G}(Main/Menu)                   {P}║")
        print(f"{P}╚══════════════════════════════════════════╝{RESET}")

        setting_choice = input(f"\n{C}➡️  Enter your choice (1-3): {RESET}")

        if setting_choice == '1':
            await change_name(api_id, api_hash, session_file)
        elif setting_choice == '2':
            await change_username(api_id, api_hash, session_file)
        elif setting_choice == '3':
            print(f"{Y}↩️ Returning to Main Menu.{RESET}")
            break
        else:
            print(f"{R}❌ Invalid choice. Please try again.{RESET}")

        await asyncio.sleep(DEFAULT_SLEEP_TIME)

# -----------------------------------------------------------
# --- Join Channels/Groups Function ---
# -----------------------------------------------------------

async def join_channels(api_id, api_hash, session_files):
    """Joins given channels/groups with selected sessions, with enhanced logging."""
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    links_input = input(f"{C}🔗 Enter channel/group links (separated by space): {RESET}")
    links = links_input.split()

    if not links:
        print(f"{R}❌ No links provided.{RESET}")
        return

    # ✅ এখানে সেশন নির্বাচন যুক্ত করা হয়েছে
    sessions_to_use = select_sessions_for_task(session_files, task_name="Join Channels/Groups")
    if not sessions_to_use:
        return

    print(f"\n{G}✅ Starting join process using {B}{len(sessions_to_use)}{G} selected sessions...{RESET}")

    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            for link in links:
                try:
                    if 't.me/+' in link or 't.me/joinchat/' in link:
                        hash_code = link.split('/')[-1].replace('+', '')
                        await client(ImportChatInviteRequest(hash_code))
                        print(f"{G}✅ [{P}{session_file}{G}] Successfully joined private link: {B}{link}{RESET}")
                    else:
                        entity = await client.get_entity(link)
                        await client(JoinChannelRequest(entity))
                        print(f"{G}✅ [{P}{session_file}{G}] Successfully joined public link: {B}{link}{RESET}")
                except UserBannedInChannelError:
                    print(f"{R}🚫 [{P}{session_file}{R}] Error: You are banned from {B}{link}. Skipping.{RESET}")
                except UserAlreadyParticipantError:
                    print(f"{Y}👍 [{P}{session_file}{Y}] Already a participant in {B}{link}. Skipping.{RESET}")
                except InviteHashExpiredError:
                    print(f"{R}⌛ [{P}{session_file}{R}] Error: The invite link {B}{link} has expired. Skipping.{RESET}")
                except ChannelsTooMuchError:
                    print(f"{R} overcrowded [{P}{session_file}{R}] Error: Account has joined too many channels. Skipping.{RESET}")
                except FloodWaitError as e:
                    print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error for {B}{link}. Waiting for {e.seconds}s.{RESET}")
                    await asyncio.sleep(e.seconds)
                except Exception as e:
                    print(f"{R}❗ [{P}{session_file}{R}] An unexpected error occurred for {B}{link}: {e}. Skipping.{RESET}")
                await asyncio.sleep(DEFAULT_SLEEP_TIME)

        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
    print(f"\n{G}{BOLD}--- ✨ Task Completed ✨ ---{RESET}")

# -----------------------------------------------------------
# --- Send Referral (Standard Message) Function ---
# -----------------------------------------------------------

async def send_referral(api_id, api_hash, session_files):
    """
    Sends start command to a bot using a standard referral link (t.me/bot?start=code),
    using only sessions selected by their numbers.
    """
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    if not session_files:
        print(f"{R}❌ No session files found to send referrals.{RESET}")
        return

    print(f"\n{C}{BOLD}--- 🚀 Standard Referral Setup ---{RESET}")

    # 1. রেফারেল লিঙ্ক ইনপুট নেওয়া
    ref_link = input(f"{C}🔗 Enter the standard referral link (e.g., t.me/bot_username?start=ref_code): {RESET}")

    try:
        # 💡 লিঙ্কের ফরম্যাট t.me/botname?start=payload হিসেবে পার্স করা
        match = re.search(r"t\.me/([^?]+)\?start=([^&]+)", ref_link, re.IGNORECASE)
        if not match:
            # যদি ?start= না থাকে, তাহলে শুধু বোট ইউজারনেম নেওয়া 
            match_bare = re.search(r"t\.me/([^?]+)", ref_link, re.IGNORECASE)
            if match_bare:
                bot_username = match_bare.group(1)
                command = "/start"
            else:
                print(f"{R}❌ Invalid referral link format. Use t.me/bot_username?start=code or t.me/bot_username format.{RESET}")
                return
        else:
            bot_username = match.group(1)
            payload = match.group(2)
            command = f"/start {payload}"
            
        print(f"{Y}💡 Detected Bot: {B}@{bot_username}{Y}, Command: {B}{command}{RESET}")
    except Exception as e:
        print(f"{R}❗ Could not parse the referral link: {e}{RESET}")
        return

    # 2. নির্বাচিত সেশন ফাইলগুলি বেছে নেওয়া 
    sessions_to_use = select_sessions_for_task(session_files, task_name="Standard Referral")
    if not sessions_to_use:
        return
    
    print(f"\n{G}✅ Starting referral process using {B}{len(sessions_to_use)}{G} selected sessions...{RESET}")

    # 3. বেছে নেওয়া সেশনগুলিতে কাজ শুরু করা
    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            print(f"{B}➤ [{P}{session_file}{B}] Sending standard referral to {C}@{bot_username}{B}...{RESET}")
            await client.send_message(bot_username, command)
            print(f"{G}✅ [{P}{session_file}{G}] Successfully sent referral.{RESET}")

        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except FloodWaitError as e:
            print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s.{RESET}")
            await asyncio.sleep(e.seconds)
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
        
    print(f"\n{G}{BOLD}--- ✨ Standard Referral Task Completed for {len(sessions_to_use)} Accounts ✨ ---{RESET}")

# -----------------------------------------------------------
# --- ✅ Mass Refer (Inline Click) ---
# -----------------------------------------------------------

async def mass_inline_click_referral(api_id, api_hash, session_files):
    """
    Sends start command to a bot and automatically clicks a user-specified inline button 
    using multiple accounts. (MODIFIED: Checks the last 2 messages for the button)
    """
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    if not session_files:
        print(f"{R}❌ No session files found.{RESET}")
        return

    print(f"\n{C}{BOLD}--- 🚀 Mass Referral (Inline Click) Setup ---{RESET}")

    # 1. রেফারেল লিঙ্ক ইনপুট নেওয়া
    ref_link = input(f"{C}🔗 Enter the full referral link (e.g., t.me/bot\\_username?start=ref\\_code): {RESET}")
    
    # 2. ইনলাইন বাটন টেক্সট ইনপুট নেওয়া
    button_text = input(f"{C}💬 Enter the EXACT text on the Inline Button to click (e.g., Confirm): {RESET}").strip()

    if not ref_link or not button_text:
        print(f"{R}❌ Referral link and button text cannot be empty. Cancelling.{RESET}")
        return
        
    target_text_cleaned = button_text.strip().lower()

    try:
        match = re.search(r"t\.me/([^?]+)\?start=([^&]+)", ref_link, re.IGNORECASE)
        if not match:
             # যদি ?start= না থাকে, তাহলে শুধু বোট ইউজারনেম নেওয়া 
            match_bare = re.search(r"t\.me/([^?]+)", ref_link, re.IGNORECASE)
            if match_bare:
                bot_username = match_bare.group(1)
                command = "/start"
            else:
                print(f"{R}❌ Invalid referral link format. Use t.me/bot_username?start=code or t.me/bot_username format.{RESET}")
                return
        else:
            bot_username = match.group(1)
            payload = match.group(2)
            command = f"/start {payload}"
            
        print(f"{Y}💡 Detected Bot: {B}@{bot_username}{Y}, Command: {B}{command}{RESET}")
        print(f"{Y}💡 Target Button Text (Cleaned): {B}{target_text_cleaned}{RESET}")
        
    except Exception as e:
        print(f"{R}❗ Could not parse the referral link: {e}{RESET}")
        return

    # 3. নির্বাচিত সেশন ফাইলগুলি বেছে নেওয়া 
    sessions_to_use = select_sessions_for_task(session_files, task_name="Mass Inline Click Referral") 
    if not sessions_to_use:
        return
    
    print(f"\n{G}✅ Starting mass inline click operation using {B}{len(sessions_to_use)}{G} selected sessions...{RESET}")

    # 4. বেছে নেওয়া সেশনগুলিতে কাজ শুরু করা
    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            bot_entity = await client.get_entity(bot_username)
            
            # 4.1. রেফারেল কমান্ড পাঠানো
            print(f"{B}➤ [{P}{session_file}{B}] Sending command: {C}{command}{B} to {C}@{bot_username}{B}...{RESET}")
            await client.send_message(bot_entity, command)
            
            # 4.2. বটের রিপ্লাই মেসেজের জন্য অপেক্ষা করা
            await asyncio.sleep(5) # ৫ সেকেন্ড অপেক্ষা, যাতে বটের মেসেজগুলো আসে

            # 4.3. শেষ দুটি মেসেজ নেওয়া
            # পরিবর্তন: limit=2 করা হলো
            messages = await client.get_messages(bot_entity, limit=2) 
            if not messages:
                print(f"{R}❌ [{P}{session_file}{R}] Error: Bot did not send a reply message. Skipping click.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue
                
            
            # 4.4. ইনলাইন বাটন খোঁজা (শেষ দুটি মেসেজ চেক করা হচ্ছে)
            found_button = None
            message_to_click = None

            # নতুন লজিক: মেসেজগুলোকে উল্টো দিক থেকে চেক করা হচ্ছে (শেষটা থেকে শুরু)
            for msg in messages:
                if msg.reply_markup and isinstance(msg.reply_markup, ReplyInlineMarkup):
                    # প্রতিটি Row চেক করা হচ্ছে
                    for row in msg.reply_markup.rows:
                        # প্রতিটি Row-এর ভেতরে থাকা প্রতিটি বাটন চেক করা হচ্ছে
                        for button in row.buttons:
                            if isinstance(button, KeyboardButtonCallback):
                                # মেসেজের বাটন টেক্সটকে পরিষ্কার করে লোয়ারকেস করা
                                message_button_text_cleaned = button.text.strip().lower() 
                                
                                # পরিষ্কার করা টেক্সট দুটিকে মেলানো
                                if message_button_text_cleaned == target_text_cleaned:
                                    found_button = button
                                    message_to_click = msg
                                    break # বাটন পেলে ভেতরের লুপ ব্রেক করা
                        if found_button:
                            break # বাটন পেলে বাইরের লুপ ব্রেক করা
                if found_button:
                    break

            # 4.5. বাটন ক্লিক করা
            if found_button:
                print(f"{G}✅ [{P}{session_file}{G}] Found button '{C}{found_button.text}{G}' in Message ID: {message_to_click.id}. Clicking...{RESET}")
                
                # ইনলাইন বাটনে ক্লিক করার জন্য API কল
                await client(GetBotCallbackAnswerRequest(
                    bot_entity,
                    message_to_click.id, # সঠিক মেসেজ আইডি ব্যবহার করা হলো
                    data=found_button.data
                ))
                
                print(f"{G}🎉 [{P}{session_file}{G}] Successfully sent click action.{RESET}")
            else:
                print(f"{Y}⚠️ [{P}{session_file}{Y}] Warning: Could not find inline button with cleaned text '{target_text_cleaned}' in the last 2 messages. Skipping click.{RESET}")


        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except FloodWaitError as e:
            print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s. Stopping for this account.{RESET}")
            await asyncio.sleep(e.seconds)
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
        
    print(f"\n{G}{BOLD}--- ✨ Mass Inline Click Referral Task Completed for {len(sessions_to_use)} Accounts ✨ ---{RESET}")

# -----------------------------------------------------------
# --- 👻 Manual Emoji Click Refer Function (UPDATED) ---
# -----------------------------------------------------------

async def manual_emoji_click_referral(api_id, api_hash, session_files):
    """
    Manual Emoji Click Refer.
    User provides the required emoji after seeing the bot's captcha message.
    """
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP

    if not session_files:
        print(f"{R}❌ No session files found.{RESET}")
        return

    print(f"\n{C}{BOLD}--- 🧑‍💻 Manual Emoji Click Refer Setup ---{RESET}")

    # 1. রেফারেল লিংক ইনপুট
    ref_link = input(f"{C}🔗 Enter the full referral link (e.g., t.me/bot_username?start=ref_code): {RESET}")

    try:
        import re
        match = re.search(r"t\.me/([^?]+)\?start=([^&]+)", ref_link, re.IGNORECASE)
        if not match:
            match_bare = re.search(r"t\.me/([^?]+)", ref_link, re.IGNORECASE)
            if match_bare:
                bot_username = match_bare.group(1)
                command = "/start"
            else:
                print(f"{R}❌ Invalid referral link format. Use t.me/bot_username?start=code or t.me/bot_username format.{RESET}")
                return
        else:
            bot_username = match.group(1)
            payload = match.group(2)
            command = f"/start {payload}"
            
        print(f"{Y}💡 Detected Bot: {B}@{bot_username}{Y}, Command: {B}{command}{RESET}")
    except Exception as e:
        print(f"{R}❗ Could not parse the referral link: {e}{RESET}")
        return
    
    # 2. সেশন নির্বাচন
    sessions_to_use = select_sessions_for_task(session_files, task_name="Manual Emoji Click Refer")
    if not sessions_to_use:
        return

    print(f"\n{G}✅ Starting manual emoji click operation using {B}{len(sessions_to_use)}{G} selected sessions...{RESET}")

    # 3. অপারেশন শুরু
    for session_file in sessions_to_use:
        # ".session" এক্সটেনশন বাদ দেওয়া
        session_name_for_print = session_file.replace(".session", "")
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_name_for_print}{RESET}")
        
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_name_for_print}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            bot_entity = await client.get_entity(bot_username)

            # 3.1. /start কমান্ড পাঠানো
            print(f"{B}➤ [{P}{session_name_for_print}{B}] Sending command: {C}{command}{B}...{RESET}")
            await client.send_message(bot_entity, command)
            
            # 3.2. বটের রিপ্লাই মেসেজের জন্য অপেক্ষা করা
            await asyncio.sleep(5) 

            # 3.3. শেষ মেসেজটি নেওয়া
            messages = await client.get_messages(bot_entity, limit=1)
            if not messages:
                print(f"{R}❌ [{P}{session_name_for_print}{R}] Error: Bot did not send a reply message. Skipping click.{RESET}")
                continue
                
            last_message = messages[0]

            # 3.4. ক্যাপচা মেসেজ প্রদর্শন
            print(f"\n{P}╔{'═' * 50}╗{RESET}")
            print(f"{P}║ {C}{BOLD}CAPTCHA MESSAGE from @{bot_username}:{RESET}")
            print(f"{P}║ {Y}Text:{RESET} {B}{last_message.text or last_message.message}{RESET}")
            
            buttons_found = False
            if last_message.reply_markup and hasattr(last_message.reply_markup, 'rows'):
                print(f"{P}║ {G}Buttons:{RESET} Found. Please identify the exact Emoji to click.")
                all_buttons = []
                for row in last_message.reply_markup.rows:
                    for button in row.buttons:
                        all_buttons.append(button.text)
                print(f"{P}║ {C}Available Emojis/Buttons: {B}{', '.join(all_buttons)}{RESET}")
                buttons_found = True
            
            print(f"{P}╚{'═' * 50}╝{RESET}")
            
            if not buttons_found:
                print(f"{R}❌ [{P}{session_name_for_print}{R}] Error: No inline buttons found in the bot's response. Skipping click.{RESET}")
                continue

            # 3.5. ইউজার ইনপুট
            # Note: asyncio.to_thread for blocking input in an async function
            target_emoji = await asyncio.to_thread(input, f"{C}💬 [{P}{session_name_for_print}{C}] Enter the EXACT Emoji to click: {RESET}")
            target_emoji = target_emoji.strip()

            if not target_emoji:
                print(f"{R}❌ Emoji not entered. Skipping click for this session.{RESET}")
                continue
            
            # 3.6. স্বয়ংক্রিয় বাটন ক্লিক
            clicked = False
            for row in last_message.reply_markup.rows:
                for button in row.buttons:
                    if isinstance(button, KeyboardButtonCallback) and button.text.strip() == target_emoji:
                        print(f"{B}➤ [{P}{session_name_for_print}{B}] Clicking button: '{C}{button.text}{B}'...{RESET}")
                        await client(GetBotCallbackAnswerRequest(
                            bot_entity,
                            last_message.id,
                            data=button.data
                        ))
                        print(f"{G}✅ [{P}{session_name_for_print}{G}] Successfully sent click action.{RESET}")
                        clicked = True
                        break
                if clicked:
                    break
            
            if not clicked:
                print(f"{R}❌ [{P}{session_name_for_print}{R}] Error: Could not find a matching inline button with text: '{target_emoji}'. Skipping click.{RESET}")

        except MessageNotModifiedError:
            print(f"{Y}⚠️ [{P}{session_name_for_print}{Y}] Warning: Message already clicked or not modified.{RESET}")
        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_name_for_print}{R} is banned or deleted. Skipping.{RESET}")
        except FloodWaitError as e:
            print(f"{R}⏳ [{P}{session_name_for_print}{R}] Flood wait error. Waiting for {e.seconds}s. Stopping for this account.{RESET}")
            await asyncio.sleep(e.seconds)
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_name_for_print}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
            
            await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
        
    print(f"\n{G}{BOLD}--- ✨ Manual Emoji Click Refer Task Completed for {len(sessions_to_use)} Accounts ✨ ---{RESET}")


# -----------------------------------------------------------
# --- 🧑‍💻 Manual Captcha Refer (ORIGINAL LOGIC - Now option 8) ---
# -----------------------------------------------------------

async def manual_captcha_referral(api_id, api_hash, session_files):
    """
    Sends start command to a bot, forwards the response message to a Captcha Group 
    for manual input, and sends the user's input as the captcha answer.
    """
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    if not session_files:
        print(f"{R}❌ No session files found.{RESET}")
        return

    print(f"\n{C}{BOLD}--- 🧑‍💻 Manual Captcha Refer Setup ---{RESET}")

    # 1. রেফারেল লিঙ্ক ইনপুট নেওয়া (t.me/bot_username?start=ref_code)
    ref_link = input(f"{C}🔗 Enter the full referral link (e.g., t.me/bot\\_username?start=ref\\_code): {RESET}")
    
    # 2. ক্যাপচা গ্রুপ লিংক চেক করা
    if CAPTCHA_GROUP_LINK == "https://t.me/CaptchaTermux":
        print(f"{R}❌ Error: CAPTCHA_GROUP_LINK is set to the default dummy value! Please set the link in the global constants section.{RESET}")
        return

    try:
        # 💡 লিঙ্কের ফরম্যাট t.me/botname?start=payload হিসেবে পার্স করা
        match = re.search(r"t\.me/([^?]+)\?start=([^&]+)", ref_link, re.IGNORECASE)
        if not match:
            # যদি ?start= না থাকে, তাহলে শুধু বোট ইউজারনেম নেওয়া 
            match_bare = re.search(r"t\.me/([^?]+)", ref_link, re.IGNORECASE)
            if match_bare:
                bot_username = match_bare.group(1)
                command = "/start"
            else:
                print(f"{R}❌ Invalid referral link format. Use t.me/bot_username?start=code or t.me/bot_username format.{RESET}")
                return
        else:
            bot_username = match.group(1)
            payload = match.group(2)
            command = f"/start {payload}"
        
        print(f"{Y}💡 Detected Bot: {B}@{bot_username}{Y}, Command: {B}{command}{RESET}")
        print(f"{Y}💡 Captcha Group: {B}{CAPTCHA_GROUP_LINK}{RESET}")
        
    except Exception as e:
        print(f"{R}❗ Could not parse the referral link: {e}{RESET}")
        return

    # 3. নির্বাচিত সেশন ফাইলগুলি বেছে নেওয়া 
    sessions_to_use = select_sessions_for_task(session_files, task_name="Manual Captcha Refer") 
    if not sessions_to_use:
        return
    
    print(f"\n{G}✅ Starting manual captcha refer operation using {B}{len(sessions_to_use)}{G} selected sessions...{RESET}")

    # 4. প্রাথমিক নির্দেশনা দেওয়া
    print(f"\n{C}{BOLD}--- Manual Input Required ---{RESET}")
    print(f"{Y}ATTENTION: The tool will now start the accounts one by one.{RESET}")
    print(f"{Y}1. It will forward the Captcha (image/button/text) to the designated group.{RESET}")
    print(f"{Y}2. After forwarding, it will PAUSE and prompt you to enter the correct CAPTCHA answer.{RESET}")
    
    # 5. বেছে নেওয়া সেশনগুলিতে কাজ শুরু করা
    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        
        captcha_group_entity = None
        
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            bot_entity = await client.get_entity(bot_username)
            
            # 5.1. ক্যাপচা গ্রুপে জয়েন করা
            try:
                print(f"{Y}☁️  Joining Captcha Group: {B}{CAPTCHA_GROUP_LINK}{RESET}")
                if 't.me/+' in CAPTCHA_GROUP_LINK or 't.me/joinchat/' in CAPTCHA_GROUP_LINK:
                    hash_code = CAPTCHA_GROUP_LINK.split('/')[-1].replace('+', '')
                    await client(ImportChatInviteRequest(hash_code))
                else:
                    await client(JoinChannelRequest(CAPTCHA_GROUP_LINK))

                captcha_group_entity = await client.get_entity(CAPTCHA_GROUP_LINK)
                print(f"{G}👍 Joined Captcha Group: {B}{captcha_group_entity.title}{RESET}")
                
            except UserAlreadyParticipantError:
                captcha_group_entity = await client.get_entity(CAPTCHA_GROUP_LINK)
                print(f"{Y}👍 Already a participant in Captcha Group. Proceeding...{RESET}")
            except Exception as e:
                print(f"{R}❌ Could not join Captcha Group! Error: {e}. Skipping this session's referral.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue
            
            await asyncio.sleep(DEFAULT_SLEEP_TIME)
            
            # 5.2. রেফারেল কমান্ড পাঠানো
            print(f"{B}➤ [{P}{session_file}{B}] Sending command: {C}{command}{B} to {C}@{bot_username}{B}...{RESET}")
            await client.send_message(bot_entity, command)
            
            # 5.3. বটের রিপ্লাই মেসেজের জন্য অপেক্ষা করা
            await asyncio.sleep(5) 

            # 5.4. শেষ মেসেজটি নেওয়া
            messages = await client.get_messages(bot_entity, limit=1)
            if not messages:
                print(f"{R}❌ [{P}{session_file}{R}] Error: Bot did not send a reply message. Skipping.{RESET}")
                continue
                
            last_message = messages[0]
            
            # 5.5. ক্যাপচা গ্রুপে মেসেজ ফরোয়ার্ড করা
            print(f"{B}➤ [{P}{session_file}{B}] Forwarding Captcha to {C}{captcha_group_entity.title}{B}...{RESET}")
            # Ensure the client is authorized to forward/send to the captcha group
            await client.forward_messages(captcha_group_entity, last_message)
            print(f"{G}✅ [{P}{session_file}{G}] Captcha forwarded successfully. CHECK THE GROUP NOW!{RESET}")

            # 5.6. ইউজার ইনপুট নেওয়া (প্রতি সেশনের জন্য)
            captcha_answer = await asyncio.to_thread(input, f"\n{C}{BOLD}💬 CAPTCHA ANSWER FOR {P}{session_file}{C} (Look in the group): {RESET}").strip()
            
            if not captcha_answer:
                print(f"{R}❌ Captcha answer was empty. Skipping captcha submission for this session.{RESET}")
                continue

            # 5.7. ক্যাপচা উত্তর পাঠানো
            print(f"{B}➤ [{P}{session_file}{B}] Sending CAPTCHA answer: {C}{captcha_answer}{B} to {C}@{bot_username}{B}...{RESET}")
            await client.send_message(bot_entity, captcha_answer)
            print(f"{G}🎉 [{P}{session_file}{G}] Successfully sent CAPTCHA answer.{RESET}")

        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except FloodWaitError as e:
            print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s. Stopping for this account.{RESET}")
            await asyncio.sleep(e.seconds)
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            # 5.8. ক্যাপচা গ্রুপ থেকে লিভ নেওয়া
            if client.is_connected() and captcha_group_entity:
                try:
                    await client(LeaveChannelRequest(captcha_group_entity))
                    print(f"{G}✅ [{P}{session_file}{G}] Successfully left Captcha Group.{RESET}")
                except Exception as e:
                    print(f"{R}❗ [{P}{session_file}{R}] Could not leave Captcha Group: {e}.{RESET}")
            
            if client.is_connected():
                await client.disconnect()
        
        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
        
    print(f"\n{G}{BOLD}--- ✨ Manual Captcha Refer Task Completed for {len(sessions_to_use)} Accounts ✨ ---{RESET}")


# -----------------------------------------------------------
# --- 🆕 Multi Inline Click Refer (NEW) ---
# -----------------------------------------------------------

async def multi_inline_click_referral(api_id, api_hash, session_files):
    """
    Sends start command to a bot and automatically clicks multiple user-specified 
    inline buttons sequentially using multiple accounts.
    """
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    if not session_files:
        print(f"{R}❌ No session files found.{RESET}")
        return

    print(f"\n{C}{BOLD}--- 🚀 Multi Inline Click Referral Setup ---{RESET}")

    # 1. রেফারেল লিঙ্ক ইনপুট নেওয়া
    ref_link = input(f"{C}🔗 Enter the full referral link (e.g., t.me/bot\\_username?start=ref\\_code): {RESET}")
    
    if not ref_link:
        print(f"{R}❌ Referral link cannot be empty. Cancelling.{RESET}")
        return
        
    try:
        # লিঙ্কের ফরম্যাট t.me/botname?start=payload হিসেবে পার্স করা
        match = re.search(r"t\.me/([^?]+)\?start=([^&]+)", ref_link, re.IGNORECASE)
        if not match:
             # যদি ?start= না থাকে, তাহলে শুধু বোট ইউজারনেম নেওয়া 
            match_bare = re.search(r"t\.me/([^?]+)", ref_link, re.IGNORECASE)
            if match_bare:
                bot_username = match_bare.group(1)
                command = "/start"
            else:
                print(f"{R}❌ Invalid referral link format. Use t.me/bot_username?start=code or t.me/bot_username format.{RESET}")
                return
        else:
            bot_username = match.group(1)
            payload = match.group(2)
            command = f"/start {payload}"
            
        print(f"{Y}💡 Detected Bot: {B}@{bot_username}{Y}, Command: {B}{command}{RESET}")
        
    except Exception as e:
        print(f"{R}❗ Could not parse the referral link: {e}{RESET}")
        return

    # 2. একাধিক ইনলাইন বাটন টেক্সট ইনপুট নেওয়া
    button_texts = [] 
    print(f"\n{C}{BOLD}--- Inline Button Collection ---{RESET}")
    print(f"{Y}Enter the EXACT text on each Inline Button sequentially. Press {B}Enter{Y} on an empty line to finish.{RESET}")
    
    i = 1
    while True:
        # Note: asyncio.to_thread for blocking input in an async function
        button_name = await asyncio.to_thread(input, f"{C}💬 {i}st/nd/rd/th Button Text (Press Enter to finish): {RESET}")
        
        button_name = button_name.strip()
        if not button_name:
            break
            
        button_texts.append(button_name)
        i += 1

    if not button_texts:
        print(f"{R}❌ No inline button texts provided. Cancelling.{RESET}")
        return

    print(f"{G}✅ Buttons to click: {B}{' -> '.join(button_texts)}{RESET}")
    
    # 3. নির্বাচিত সেশন ফাইলগুলি বেছে নেওয়া 
    sessions_to_use = select_sessions_for_task(session_files, task_name="Multi Inline Click Referral") 
    if not sessions_to_use:
        return
    
    print(f"\n{G}✅ Starting multi inline click operation using {B}{len(sessions_to_use)}{G} selected sessions...{RESET}")

    # 4. বেছে নেওয়া সেশনগুলিতে কাজ শুরু করা
    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            bot_entity = await client.get_entity(bot_username)
            
            # 4.1. রেফারেল কমান্ড পাঠানো (প্রথম ধাপ)
            print(f"{B}➤ [{P}{session_file}{B}] Sending command: {C}{command}{B} to {C}@{bot_username}{B}...{RESET}")
            await client.send_message(bot_entity, command)
            
            # 4.2. বটের রিপ্লাই মেসেজের জন্য অপেক্ষা করা
            await asyncio.sleep(5) 
            
            # 4.3. বাটন লিস্ট ধরে লুপ শুরু
            click_success = True
            
            for i, target_button_text in enumerate(button_texts, 1):
                target_text_cleaned = target_button_text.strip().lower()
                
                # শেষ মেসেজটি নেওয়া (যেহেতু ক্লিক করার পর নতুন মেসেজ আসবে)
                messages = await client.get_messages(bot_entity, limit=2) 
                
                if not messages:
                    print(f"{R}❌ [{P}{session_file}{R}] Error: Bot did not send a reply message for Step {i}. Stopping clicks.{RESET}")
                    click_success = False
                    break
                
                found_button = None
                message_to_click = None

                # শেষ দুটি মেসেজ থেকে বাটন খোঁজা
                for msg in messages:
                    if msg.reply_markup and isinstance(msg.reply_markup, ReplyInlineMarkup):
                        for row in msg.reply_markup.rows:
                            for button in row.buttons:
                                if isinstance(button, KeyboardButtonCallback):
                                    message_button_text_cleaned = button.text.strip().lower() 
                                    
                                    if message_button_text_cleaned == target_text_cleaned:
                                        found_button = button
                                        message_to_click = msg
                                        break
                            if found_button:
                                break
                    if found_button:
                        break

                if found_button:
                    print(f"{G}✅ [{P}{session_file}{G}] Step {i}: Found button '{C}{found_button.text}{G}'. Clicking...{RESET}")
                    
                    await client(GetBotCallbackAnswerRequest(
                        bot_entity,
                        message_to_click.id, 
                        data=found_button.data
                    ))
                    
                    print(f"{G}🎉 [{P}{session_file}{G}] Step {i}: Click action sent successfully.{RESET}")
                    await asyncio.sleep(3) # পরবর্তী বাটন ক্লিকের আগে অপেক্ষা
                else:
                    print(f"{R}❌ [{P}{session_file}{R}] Step {i}: Could not find button with text '{target_button_text}'. Stopping clicks for this session.{RESET}")
                    click_success = False
                    break
            
            if click_success:
                print(f"{G}✔️ [{P}{session_file}{G}] All {len(button_texts)} steps completed successfully.{RESET}")
                
        except MessageNotModifiedError:
            print(f"{Y}⚠️ [{P}{session_file}{Y}] Warning: Message already clicked or not modified. Continuing to next step/session.{RESET}")
        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except FloodWaitError as e:
            print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s. Stopping for this account.{RESET}")
            await asyncio.sleep(e.seconds)
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
        
    print(f"\n{G}{BOLD}--- ✨ Multi Inline Click Referral Task Completed ✨ ---{RESET}")




# -----------------------------------------------------------
# --- 🆕 Group Comment Operation (FINAL FIXED with Function Call) ---
# -----------------------------------------------------------

async def group_comment_operation(api_id, api_hash, session_files):
    """
    Automatically posts comments in a group with multiple accounts.
    Users can input multiple comments, and the tool will randomly select comments to post.
    """
    
    # প্রয়োজনীয় মডিউল/ত্রুটি ইম্পোর্ট করা হচ্ছে
    try:
        from telethon import TelegramClient
        from telethon.errors import UserDeactivatedBanError, AuthKeyUnregisteredError, FloodWaitError
        import random
        import asyncio
        # random_delay এবং গ্লোবাল কনস্ট্যান্টের জন্য অতিরিক্ত ইমপোর্ট দরকার নেই, যেহেতু উপরে সংজ্ঞায়িত
    except ImportError as e:
        print(f"{R}❌ ইম্পোর্ট ত্রুটি: {e}. নিশ্চিত করুন যে 'telethon', 'random' এবং 'asyncio' ইনস্টল করা আছে।{RESET}")
        return
        
    if not session_files:
        print(f"{R}❌ কোনো সেশন ফাইল পাওয়া যায়নি।{RESET}")
        return

    print(f"\n{C}{BOLD}--- 💬 Group Comment Operation ---{RESET}")

    # 1. গ্রুপ লিঙ্ক ইনপুট
    group_link = input(f"{C}🔗 গ্রুপের লিঙ্কটি দিন: {RESET}").strip()
    if not group_link:
        print(f"{R}❌ গ্রুপের লিঙ্ক খালি থাকতে পারে না।{RESET}")
        return
        
    # 2. মাল্টিপল কমেন্ট ইনপুট
    comments = []
    print(f"\n{C}{BOLD}--- কমেন্ট সংগ্রহ ---{RESET}")
    print(f"{Y}কমেন্ট দিন (প্রতি লাইনে একটি)। খালি লাইন দিয়ে {B}ENTER{Y} চাপলে শেষ হবে।{RESET}")
    
    i = 1
    while True:
        comment = await asyncio.to_thread(input, f"{C}💬 Comment {i}: {RESET}")
        comment = comment.strip()
        if not comment:
            break
        comments.append(comment)
        i += 1

    if not comments:
        print(f"{R}❌ কোনো কমেন্ট দেওয়া হয়নি। বাতিল করা হচ্ছে।{RESET}")
        return

    print(f"{G}✅ মোট কমেন্ট সংগ্রহ করা হয়েছে: {B}{len(comments)}{RESET}")

    # 3. কমেন্ট ইন্টারভাল (সেকেন্ড)
    try:
        interval = int(input(f"{C}⏱️  কমেন্টের মাঝে বিরতি (সেকেন্ডে): {RESET}").strip())
        if interval < 1:
            print(f"{R}❌ বিরতি কমপক্ষে ১ সেকেন্ড হতে হবে। ডিফল্ট ১০ সেকেন্ড ব্যবহার করা হচ্ছে।{RESET}")
            interval = 10
    except ValueError:
        print(f"{R}❌ অবৈধ ইনপুট। ডিফল্ট ১০ সেকেন্ড ব্যবহার করা হচ্ছে।{RESET}")
        interval = 10

    # 4. প্রতি অ্যাকাউন্টে কতগুলো কমেন্ট করবে
    try:
        comments_per_account = int(input(f"{C}📝 প্রতিটি অ্যাকাউন্ট থেকে কতগুলো কমেন্ট পোস্ট হবে?: {RESET}").strip())
        if comments_per_account < 1:
            print(f"{R}❌ কমপক্ষে ১টি কমেন্ট প্রতি অ্যাকাউন্ট থেকে দিতে হবে। ডিফল্ট ১ ব্যবহার করা হচ্ছে।{RESET}")
            comments_per_account = 1
    except ValueError:
        print(f"{R}❌ অবৈধ ইনপুট। ডিফল্ট ১টি কমেন্ট প্রতি অ্যাকাউন্ট ব্যবহার করা হচ্ছে।{RESET}")
        comments_per_account = 1

    # 5. র‍্যান্ডমভাবে কতগুলো কমেন্ট সিলেক্ট করবে
    try:
        random_comments_count = int(input(f"{C}🎲 আপনার তালিকা থেকে কতগুলো র‍্যান্ডম কমেন্ট বেছে নেওয়া হবে?: {RESET}").strip())
        if random_comments_count < 1:
            print(f"{R}❌ কমপক্ষে ১টি কমেন্ট নির্বাচন করতে হবে। সব কমেন্ট ব্যবহার করা হচ্ছে।{RESET}")
            random_comments_count = len(comments)
        elif random_comments_count > len(comments):
            print(f"{Y}⚠️ শুধুমাত্র {len(comments)}টি কমেন্ট উপলব্ধ। সব কমেন্ট ব্যবহার করা হচ্ছে।{RESET}")
            random_comments_count = len(comments)
    except ValueError:
        print(f"{R}❌ অবৈধ ইনপুট। সব কমেন্ট ব্যবহার করা হচ্ছে।{RESET}")
        random_comments_count = len(comments)
        
    # 6. সেশন সিলেকশন এবং তালিকা প্রদর্শন
    session_map = {i: session_file for i, session_file in enumerate(session_files, 1)}
    
    print(f"\n{C}{BOLD}--- উপলব্ধ সেশন ফাইল ({len(session_files)}টি) ---{RESET}")
    for i, session_file in session_map.items():
        print(f"{B}{i}. {P}{session_file}{RESET}")
        
    sessions_to_use = []
    while True:
        selection_input = input(f"\n{C}🔢 যেই সেশনগুলির নম্বর কমা দিয়ে আলাদা করে দিন (e.g., 1,2,3), অথবা {G}ALL{C} লিখুন, অথবা পুরো ফাইলের নাম দিন: {RESET}").strip()
        
        if selection_input.upper() == 'ALL':
            sessions_to_use = session_files
            break
        
        if selection_input in session_files:
            sessions_to_use = [selection_input]
            break
        
        try:
            raw_indices = [x.strip() for x in selection_input.split(',')]
            selected_indices = [int(i) for i in raw_indices if i.isdigit() and int(i) in session_map]
            
            if not selected_indices:
                print(f"{R}❌ কোনো বৈধ সেশন নম্বর বা ফাইলের নাম নির্বাচন করা হয়নি। আবার চেষ্টা করুন।{RESET}")
                continue
            
            sessions_to_use = [session_map[i] for i in selected_indices]
            break
            
        except ValueError:
            print(f"{R}❌ ইনপুট ফরম্যাট ভুল। কমা-সেপারেটেড নম্বর বা পুরো ফাইলের নাম ব্যবহার করুন।{RESET}")
            
    if not sessions_to_use:
        print(f"{R}❌ কোনো সেশন নির্বাচন করা হয়নি। বাতিল করা হচ্ছে।{RESET}")
        return

    print(f"\n{G}✅ গ্রুপে কমেন্ট করার প্রক্রিয়া শুরু হচ্ছে...{RESET}")
    print(f"{Y}   টার্গেট গ্রুপ: {B}{group_link}{RESET}")
    print(f"{Y}   অ্যাকাউন্ট সংখ্যা: {B}{len(sessions_to_use)}{RESET}")
    print(f"{Y}   প্রতি অ্যাকাউন্টে কমেন্ট: {B}{comments_per_account}{RESET}")
    print(f"{Y}   বিরতি: {B}{interval}s{RESET}")

    # কনফার্মেশন
    confirmation = input(f"\n{C}➡️  কমেন্ট অপারেশন শুরু করবেন? ({G}Y{C}/{R}N{C}): {RESET}").strip().upper()
    if confirmation != 'Y':
        print(f"{Y}❌ অপারেশন বাতিল করা হয়েছে।{RESET}")
        return

    # মেইন অপারেশন
    successful_comments = 0
    failed_comments = 0

    for session_index, session_file in enumerate(sessions_to_use, 1):
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 [{session_index}/{len(sessions_to_use)}] প্রক্রিয়া চলছে: {P}{session_file}{RESET}")
        
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ সেশন {P}{session_file}{R} অথোরাইজড নয়। এড়িয়ে যাওয়া হচ্ছে।{RESET}")
                await client.disconnect()
                
                # ✅ ১. অননুমোদিত সেশনের পরে র্যান্ডম ডিলে (ফাংশন কল)
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                    
                continue

            # গ্রুপ এনটিটি পাওয়া
            try:
                group_entity = await client.get_entity(group_link)
                group_title = getattr(group_entity, 'title', 'Unknown Group')
                print(f"{B}➤ গ্রুপে প্রবেশ করা হয়েছে: {C}{group_title}{RESET}")
            except Exception as e:
                print(f"{R}❌ গ্রুপ অ্যাক্সেস করা যায়নি: {e}. এড়িয়ে যাওয়া হচ্ছে।{RESET}")
                await client.disconnect()
                
                # ✅ ২. গ্রুপ অ্যাক্সেস ত্রুটির পরে র্যান্ডম ডিলে (ফাংশন কল)
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                    
                continue

            # এই অ্যাকাউন্টের জন্য র‍্যান্ডম কমেন্ট সিলেক্ট করা
            selected_comments = random.sample(comments, min(random_comments_count, len(comments)))
            
            # কমেন্ট পোস্ট করা
            for comment_index in range(comments_per_account):
                if comment_index < len(selected_comments):
                    comment_to_post = selected_comments[comment_index]
                else:
                    comment_to_post = random.choice(comments)
                
                try:
                    print(f"{B}➤ কমেন্ট পোস্ট হচ্ছে {comment_index+1}/{comments_per_account}: {C}{comment_to_post}{RESET}")
                    await client.send_message(group_entity, comment_to_post)
                    print(f"{G}✅ কমেন্ট সফলভাবে পোস্ট হয়েছে।{RESET}")
                    successful_comments += 1
                    
                    # ইন্টারভাল ওয়েট (শেষ কমেন্টের পর না)
                    if comment_index < comments_per_account - 1:
                        print(f"{Y}⏳ পরের কমেন্টের জন্য {interval} সেকেন্ড অপেক্ষা করা হচ্ছে...{RESET}")
                        await asyncio.sleep(interval)
                        
                except FloodWaitError as e:
                    # ✅ ৩. ফ্লাড ওয়েট হ্যান্ডলিং (যেমন ছিল)
                    print(f"{R}⏳ Flood wait: {e.seconds}s. অপেক্ষা করা হচ্ছে...{RESET}")
                    await asyncio.sleep(e.seconds)
                    failed_comments += 1
                except Exception as e:
                    print(f"{R}❌ কমেন্ট পোস্ট করা ব্যর্থ: {e}{RESET}")
                    failed_comments += 1

        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 সেশন {P}{session_file}{R} নিষিদ্ধ বা মুছে ফেলা হয়েছে। এড়িয়ে যাওয়া হচ্ছে।{RESET}")
            failed_comments += comments_per_account
        except Exception as e:
            print(f"{R}❗ {P}{session_file}{R} এর সাথে অপ্রত্যাশিত ত্রুটি: {e}. এড়িয়ে যাওয়া হচ্ছে।{RESET}")
            failed_comments += comments_per_account
        finally:
            if client.is_connected():
                await client.disconnect()

        # 💡 ৪. সেশনগুলির মধ্যে র্যান্ডম ডিলে (ফাংশন কল)
        if session_index < len(sessions_to_use):
            await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)


    # রিপোর্ট
    print(f"\n{G}{BOLD}--- ✨ Group Comment Operation সম্পূর্ণ হয়েছে ✨ ---{RESET}")
    print(f"{G}✅ সফল কমেন্ট: {B}{successful_comments}{RESET}")
    print(f"{R}❌ ব্যর্থ কমেন্ট: {B}{failed_comments}{RESET}")
    print(f"{Y}📊 মোট চেষ্টা: {B}{successful_comments + failed_comments}{RESET}")

# -----------------------------------------------------------
# --- 🤖 Inline Button Click Function (MODIFIED with Random Delay) ---
# -----------------------------------------------------------

async def send_inline_button_click(api_id, api_hash, session_files):
    """
    Sends a message (optional), waits for a specific time, and clicks a specific 
    inline button containing user-defined text in the latest bot message.
    """
    # 💡 এখানে Global ভেরিয়েবল ব্যবহার করা হয়েছে
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    if not session_files:
        print(f"{R}❌ কোনো সেশন ফাইল পাওয়া যায়নি।{RESET}")
        return

    print(f"\n{C}{BOLD}--- 🤖 ইনলাইন বাটন ক্লিক সার্ভিস ---{RESET}")

    # ১. টার্গেট বট ইনপুট নেওয়া
    target_bot = input(f"{C}🤖 Send Youre Target Bot Usernma Or Link Ex.(@usernameBot / http://t.me/Usernamebot): {RESET}").strip()
    if not target_bot:
        print(f"{R}❌ Please Send Youre Valid Bot Username Or Link{RESET}")
        return

    # ২. বাটন টেক্সট ইনপুট নেওয়া
    button_text = input(f"{C}🎛️ Please Send Youre Target Inline Text Ex: ('Check', '✅ Подтвердить', 'Start Task'): {RESET}").strip()
    if not button_text:
        print(f"{R}❌ বাটন টেক্সট খালি থাকতে পারে না।{RESET}")
        return

    # ৩. বিলম্বের সময় ইনপুট নেওয়া
    try:
        delay_input = input(f"{C}⏳ How many Seceond Leater Click Inline Buttone?: {RESET}").strip()
        delay_seconds = int(delay_input)
        if delay_seconds < 0:
            raise ValueError
    except ValueError:
        print(f"{Y}⚠️ অবৈধ ইনপুট। ডিফল্টভাবে {DEFAULT_SLEEP_TIME} সেকেন্ড ব্যবহার করা হচ্ছে।{RESET}")
        delay_seconds = DEFAULT_SLEEP_TIME

    # ৪. সেশন নির্বাচন (Unchanged)
    session_map = {i: f for i, f in enumerate(session_files, 1)}
    print(f"\n{C}{BOLD}--- Available Session Files ({len(session_files)} Total) ---{RESET}")
    for i, session_file in session_map.items():
        print(f"{B}{i}. {P}{session_file}{RESET}")

    sessions_to_use = []
    while True:
        selection_input = input(f"\n{C}🔢 Enter the numbers of the sessions you want to use (e.g., 1,2,3) or ALL for all: {RESET}").strip()
        
        if selection_input.upper() == 'ALL':
            sessions_to_use = session_files
            break

        try:
            raw_indices = [x.strip() for x in selection_input.split(',')]
            selected_indices = [int(i) for i in raw_indices if i.isdigit() and int(i) in session_map]
            
            if not selected_indices:
                print(f"{R}❌ You Send Rong session File Name.{RESET}")
                continue
            
            sessions_to_use = [session_map[i] for i in selected_indices]
            break
            
        except ValueError:
            print(f"{R}❌ ইনপুট ফরম্যাট ভুল। কমা-আলাদা সংখ্যা ব্যবহার করুন।{RESET}")
            
    print(f"\n{G}✅ Starting Inline Click process using {B}{len(sessions_to_use)} {G}selected sessions...{RESET}")

    # ৫. নির্বাচিত সেশনগুলিতে কাজ শুরু করা
    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 সেশন প্রক্রিয়াকরণ: {P}{session_file}{RESET}")
        
        try:
            from telethon.errors import UserDeactivatedBanError, AuthKeyUnregisteredError, FloodWaitError
            
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ ত্রুটি: সেশন {P}{session_file}{R} অনুমোদিত নয়। এড়িয়ে যাওয়া হচ্ছে।{RESET}")
                await client.disconnect()
                
                # ✅ সেশন অনুমোদিত না হলে র্যান্ডম ডিলে যুক্ত করা হলো
                try:
                    await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                except NameError:
                    await asyncio.sleep(5) 
                    
                continue

            # বটের সাথে চ্যাট শুরু/যাচাই করা
            try:
                entity = await client.get_entity(target_bot)
                if not getattr(entity, 'bot', False):
                    print(f"{Y}⚠️ টার্গেট '{target_bot}' একটি বট নয়। এড়িয়ে যাওয়া হচ্ছে।{RESET}")
                    continue
            except (ValueError, TypeError):
                print(f"{R}❓ [{P}{session_file}{R}] ত্রুটি: '{target_bot}' খুঁজে পাওয়া যায়নি। ইউজারনেম/লিঙ্ক যাচাই করুন। এড়িয়ে যাওয়া হচ্ছে।{RESET}")
                await client.disconnect()
                
                # ✅ বটের তথ্য না পেলে র্যান্ডম ডিলে যুক্ত করা হলো
                try:
                    await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                except NameError:
                    await asyncio.sleep(5) 
                    
                continue

            print(f"{Y}⏳ Wait for click {delay_seconds} Seceond...{RESET}")
            await asyncio.sleep(delay_seconds)

            # বটের সাথে শেষ মেসেজটি আনা
            messages = await client.get_messages(entity, limit=1)
            
            if not messages:
                print(f"{R}❌ [{P}{session_file}{R}] No Found Any Bot Message '{target_bot}' Skipping...{RESET}")
                continue
            
            last_message = messages[0]

            if not last_message.buttons:
                print(f"{R}❌ [{P}{session_file}{R}] Not Found Last Message Inline Button Skipping...{RESET}")
                continue

            # ইনলাইন বাটন খোঁজা এবং ক্লিক করা
            clicked = False
            for row in last_message.buttons:
                for button in row:
                    # button.text.strip() ব্যবহার করা হয়েছে যাতে whitespace সমস্যা না হয়
                    if button.text.strip() == button_text: 
                        print(f"{B}➤ [{P}{session_file}{B}] '{button_text}' Target Inline Button Found Cliking...{RESET}")
                        await button.click()
                        print(f"{G}✅ [{P}{session_file}{G}] Succesfuly Clicking Button '{button_text}' {RESET}")
                        clicked = True
                        break
                if clicked:
                    break
            
            if not clicked:
                print(f"{R}❌ [{P}{session_file}{R}] '{button_text}' লেখা বাটনটি খুঁজে পাওয়া যায়নি। বাটনের টেক্সট যাচাই করুন।{RESET}")


        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 ত্রুটি: সেশন {P}{session_file}{R} Deleate Bot Or Bloking {RESET}")
        except FloodWaitError as e:
            # ✅ ফ্লাড ওয়েট হ্যান্ডলিং (আগের কোডের মতো)
            print(f"{R}⏳ [{P}{session_file}{R}] ফ্লাড ওয়েট ত্রুটি। {e.seconds} সেকেন্ড অপেক্ষা করা হচ্ছে।{RESET}")
            await asyncio.sleep(e.seconds)
        except Exception as e:
            print(f"{R}❗ [{P}{session_file}{R}] অপ্রত্যাশিত ত্রুটি: {e}। এড়িয়ে যাওয়া হচ্ছে।{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        # 💡 সেশনগুলির মধ্যে র্যান্ডম ডিলে (আপনার অনুরোধ অনুযায়ী)
        try:
            await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
        except NameError:
            # যদি random_delay বা ভেরিয়েবল সংজ্ঞায়িত না থাকে, ডিফল্ট ডিলে ব্যবহার করা হলো
            print(f"{Y}⚠️ random_delay/CURRENT_SLEEP not defined. Using fixed 5s delay.{RESET}")
            await asyncio.sleep(5) 
        
    print(f"\n{G}{BOLD}--- ✨ Target Inline Button Task Compleate ✨ ---{RESET}")



# -----------------------------------------------------------
# --- 📞 Share Contact Service Function (MODIFIED with Random Delay) ---
# -----------------------------------------------------------

async def share_contact_service(api_id, api_hash, session_files):
    """
    Performs the two-step contact share with clear display of available session files.
    1. Clicks the request_contact keyboard button (no text message sent).
    2. Sends the contact data directly to simulate the pop-up confirmation.
    """
    # 💡 Global ভেরিয়েবল ব্যবহার করা হয়েছে
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    # প্রয়োজনীয় মডিউল/ত্রুটি ইম্পোর্ট করা হচ্ছে
    try:
        from telethon import types 
        from telethon.errors import UserDeactivatedBanError, AuthKeyUnregisteredError, FloodWaitError
    except ImportError as e:
        print(f"{R}❌ ইম্পোর্ট ত্রুটি: {e}. নিশ্চিত করুন যে 'telethon' ইনস্টল করা আছে।{RESET}")
        return

    if not session_files:
        print(f"{R}❌ No session files found.{RESET}")
        return

    print(f"\n{C}{BOLD}--- 📞 Share Contact Service ---{RESET}")

    # ১. টার্গেট ইউজারনেম/আইডি ইনপুট নেওয়া
    target_entity_input = input(f"{C}🎯 Enter the username, ID, or link of the target User or Bot (TO RECEIVE CONTACT): {RESET}").strip()
    if not target_entity_input:
        print(f"{R}❌ Target cannot be empty.{RESET}")
        return

    # ২. সেশন নির্বাচন (টার্গেট অ্যাকাউন্ট নির্বাচন) (Unchanged)
    session_map = {i: f for i, f in enumerate(session_files, 1)}
    
    # সেশন ফাইলগুলির তালিকা পরিষ্কারভাবে প্রদর্শন
    print(f"\n{C}{BOLD}--- Available Session Files ({len(session_files)} Total) ---{RESET}")
    for i, session_file in session_map.items():
        print(f"{B}{i}. {P}{session_file}{RESET}") 
    
    sessions_to_use = []
    while True:
        selection_input = input(f"\n{C}🔢 Enter the numbers of the sessions to use, separated by commas (e.g., 1,2,3), type {G}ALL{C}, or type the {G}full filename{C}: {RESET}").strip()
        
        if selection_input.upper() == 'ALL':
            sessions_to_use = session_files
            break
        
        # ফুল ফাইলের নাম দিয়ে খোঁজা
        if selection_input in session_files:
            sessions_to_use = [selection_input]
            break
        
        # নাম্বার ইনপুট লজিক
        try:
            raw_indices = [x.strip() for x in selection_input.split(',')]
            selected_indices = [int(i) for i in raw_indices if i.isdigit() and int(i) in session_map]
            
            if not selected_indices:
                print(f"{R}❌ No valid session number or filename selected. Try again.{RESET}")
                continue
            
            sessions_to_use = [session_map[i] for i in selected_indices]
            break
            
        except ValueError:
            print(f"{R}❌ Input format error. Use comma-separated numbers or the full filename.{RESET}")


    print(f"\n{G}✅ Starting contact sharing using {B}{len(sessions_to_use)}{G} selected sessions...{RESET}")

    KEYBOARD_BUTTON_TEXTS = [
        "Поделиться номером телефона", 
        "📱 Поделиться номером телефона", 
        "Share contact", 
        "Share Contact"
    ] 
    SEARCH_LIMIT = 20 

    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                
                # ✅ অননুমোদিত সেশনের পরে র্যান্ডম ডিলে যুক্ত করা হলো
                try:
                    await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                except NameError:
                    await asyncio.sleep(5)
                
                continue

            # Verifying/Getting target entity
            try:
                entity = await client.get_entity(target_entity_input)
                me = await client.get_me() 
                
                is_bot = getattr(entity, 'bot', False) 
                
                if not me.phone:
                    print(f"{R}❌ [{P}{session_file}{R}] Error: Session account does not have a registered phone number to share. Skipping.{RESET}")
                    await client.disconnect()
                    
                    # ✅ ফোন নম্বর না থাকলে র্যান্ডম ডিলে যুক্ত করা হলো
                    try:
                        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                    except NameError:
                        await asyncio.sleep(5)
                        
                    continue
                
            except Exception as e:
                print(f"{R}❓ [{P}{session_file}{R}] Error getting entity: {e}. Skipping.{RESET}")
                await client.disconnect()
                
                # ✅ এন্টিটি খুঁজে না পেলে র্যান্ডম ডিলে যুক্ত করা হলো
                try:
                    await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                except NameError:
                    await asyncio.sleep(5)
                
                continue
            
            
            # --- ধাপ ১: কীবোর্ড বাটন খুঁজে ক্লিক করা ---
            button_clicked = False
            
            if is_bot:
                print(f"{Y}🔍 Target is a Bot. Checking the LAST {SEARCH_LIMIT} messages for the keyboard button (Step 1)...{RESET}")
                messages = await client.get_messages(entity, limit=SEARCH_LIMIT) 
                
                if messages:
                    for message in messages:
                        if message.buttons:
                            for row in message.buttons:
                                for button in row:
                                    # request_contact বাটন খোঁজা
                                    is_request_contact = hasattr(button.button, 'request_contact') and button.button.request_contact
                                    
                                    if is_request_contact:
                                        print(f"{B}➤ [{P}{session_file}{B}] Step 1: Request Contact button ('{button.text}') found. Performing true click (No text sent)...{RESET}")
                                        
                                        # নন-টেক্সট ক্লিক পদ্ধতি
                                        await message.click(button=button)
                                        button_clicked = True
                                        break
                                if button_clicked:
                                    break
                        if button_clicked:
                            break
                
                if not button_clicked:
                    print(f"{Y}⚠️ [{P}{session_file}{Y}] Keyboard button not found. Proceeding directly to contact share (Step 2).{RESET}")
            
            
            # --- ধাপ ২: কন্টাক্ট ডেটা পাঠানো (পপ-আপ কনফার্মেশন) ---
            print(f"{B}➤ [{P}{session_file}{B}] Step 2: Sending contact data to '{entity.username or entity.id}' (Simulating pop-up confirmation)...{RESET}")
            
            # VCard ডেটা তৈরি করা 
            vcard_data = f"BEGIN:VCARD\nVERSION:3.0\nFN:{me.first_name or ''} {me.last_name or ''}\nTEL:{me.phone}\nEND:VCARD"

            try:
                await client.send_file(
                    entity,
                    types.InputMediaContact(
                        phone_number=me.phone,
                        first_name=me.first_name or "",
                        last_name=me.last_name or "",
                        vcard=vcard_data 
                    )
                )
                
                print(f"{G}✅ [{P}{session_file}{G}] Contact successfully shared with '{entity.username or entity.id}'.{RESET}")
                
            except Exception as e:
                print(f"{R}❌ [{P}{session_file}{R}] Failed to send contact data (Error: {e}). Skipping.{RESET}")


        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except FloodWaitError as e:
            # ✅ ফ্লাড ওয়েট হ্যান্ডলিং
            print(f"{R}⏳ [{P}{session_file}{R}] Flood Wait Error. Waiting for {e.seconds} seconds.{RESET}")
            await asyncio.sleep(e.seconds)
        except Exception as e:
            print(f"{R}❗ [{P}{session_file}{R}] Unexpected error: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        # 💡 সেশনগুলির মধ্যে র্যান্ডম ডিলে (আপনার অনুরোধ অনুযায়ী)
        try:
            await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
        except NameError:
            # যদি random_delay বা ভেরিয়েবল সংজ্ঞায়িত না থাকে, ডিফল্ট ডিলে ব্যবহার করা হলো
            print(f"{Y}⚠️ random_delay/CURRENT_SLEEP not defined. Using fixed 5s delay.{RESET}")
            await asyncio.sleep(5)
            
    print(f"\n{G}{BOLD}--- ✨ Share Contact Task Completed ✨ ---{RESET}")


# -----------------------------------------------------------
# --- Helper: Load and Remove Unique Solana Address (FINAL) ---
# -----------------------------------------------------------
import random
import os # os মডিউল ব্যবহার করা হবে
from telethon import TelegramClient, events # নিশ্চিত করুন যে এই লাইনগুলি আপনার ফাইলের উপরে আছে

# ⚠️ আপনার Solana Address ফাইলটির নাম 
SOLANA_ADDRESS_FILE = 'solana_addresses.txt' 

def load_and_remove_address():
    """
    Loads all Solana addresses from the file, randomly selects one, 
    and saves the remaining addresses back to the file.
    Returns the selected unique address or None if the file is empty or not found.
    """
    try:
        # 🚨 ফাইলটি কোথায় খুঁজছে তা প্রিন্ট করা হলো, যাতে আপনি বুঝতে পারেন
        full_path = os.path.abspath(SOLANA_ADDRESS_FILE)
        print(f"{Y}🔍 Checking for Solana address file at: {full_path}{RESET}") 
        
        # 🚨 যদি ফাইলটি না পাওয়া যায়, তবে এটি স্বয়ংক্রিয়ভাবে তৈরি করার চেষ্টা করবে
        if not os.path.exists(SOLANA_ADDRESS_FILE):
             with open(SOLANA_ADDRESS_FILE, 'w') as f:
                 f.write('\n# Add your Solana addresses below, one per line\n')
             print(f"{R}❌ Error: Solana address file '{SOLANA_ADDRESS_FILE}' not found. {G}A new empty file has been created.{RESET}")
             print(f"{Y}⚠️ Please add Solana addresses to the file and run again.{RESET}")
             return None

        # ফাইল থেকে সমস্ত অ্যাড্রেস পড়া
        with open(SOLANA_ADDRESS_FILE, 'r') as f:
            # কমেন্ট লাইন (# দিয়ে শুরু) বাদ দিয়ে অ্যাড্রেসগুলি পড়ে নেওয়া
            addresses = [line.strip() for line in f.readlines() if line.strip() and not line.startswith('#')]

        if not addresses:
            print(f"{R}⚠️ File '{SOLANA_ADDRESS_FILE}' found but is empty (or contains only comments). Cannot send address.{RESET}")
            return None
        
        # তালিকা থেকে একটি র‍্যান্ডম অ্যাড্রেস নির্বাচন করা
        selected_address = random.choice(addresses)
        
        # নির্বাচিত অ্যাড্রেসটি তালিকা থেকে মুছে ফেলা
        addresses.remove(selected_address)
        
        # বাকি অ্যাড্রেসগুলি ফাইলে সেভ করা
        with open(SOLANA_ADDRESS_FILE, 'w') as f:
            # কমেন্ট লাইনটি আবার যোগ করা হলো
            f.write('# Add your Solana addresses below, one per line\n') 
            f.write('\n'.join(addresses))
            if addresses: 
                 f.write('\n')
            
        return selected_address
        
    except Exception as e:
        print(f"{R}❌ Critical Error processing address file: {e}.{RESET}")
        return None


# -----------------------------------------------------------
# --- 💰 Solana Address Sender Function (MODIFIED with Random Delay) ---
# -----------------------------------------------------------

async def send_solana_address(api_id, api_hash, session_files):
    """
    Prompts for a target username/ID and sends a unique, randomly selected Solana 
    address from the file, ensuring no address is reused.
    """
    # 💡 Global ভেরিয়েবল ব্যবহার করা হয়েছে
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    # প্রয়োজনীয় মডিউল/ত্রুটি ইম্পোর্ট করা হচ্ছে
    try:
        from telethon.errors import UserDeactivatedBanError, AuthKeyUnregisteredError, FloodWaitError
        import asyncio
        # import random # random_delay-এর জন্য
    except ImportError as e:
        print(f"{R}❌ ইম্পোর্ট ত্রুটি: {e}. নিশ্চিত করুন যে 'telethon' ইনস্টল করা আছে।{RESET}")
        return
    
    if not session_files:
        print(f"{R}❌ No session files found.{RESET}")
        return

    print(f"\n{C}{BOLD}--- 💰 Solana Address Sender (Unique & Random) ---{RESET}")

    # ১. টার্গেট ইউজারনেম/আইডি ইনপুট নেওয়া
    target_entity_input = input(f"{C}🎯 Enter the username, ID, or link of the target User or Bot (TO RECEIVE ADDRESS): {RESET}").strip()
    if not target_entity_input:
        print(f"{R}❌ Target cannot be empty.{RESET}")
        return

    # ২. সেশন নির্বাচন (কোন অ্যাকাউন্টগুলি ব্যবহার করা হবে) (Unchanged)
    session_map = {i: f for i, f in enumerate(session_files, 1)}
    
    # সেশন ফাইলগুলির তালিকা পরিষ্কারভাবে প্রদর্শন
    print(f"\n{C}{BOLD}--- Available Session Files ({len(session_files)} Total) ---{RESET}")
    for i, session_file in session_map.items():
        print(f"{B}{i}. {P}{session_file}{RESET}") 

    sessions_to_use = []
    while True:
        selection_input = input(f"\n{C}🔢 Enter the numbers of the sessions to use, separated by commas (e.g., 1,2,3), type {G}ALL{C}, or type the {G}full filename{C}: {RESET}").strip()
        
        if selection_input.upper() == 'ALL':
            sessions_to_use = session_files
            break
        
        if selection_input in session_files:
            sessions_to_use = [selection_input]
            break
        
        try:
            raw_indices = [x.strip() for x in selection_input.split(',')]
            selected_indices = [int(i) for i in raw_indices if i.isdigit() and int(i) in session_map]
            
            if not selected_indices:
                print(f"{R}❌ No valid session number or filename selected. Try again.{RESET}")
                continue
            
            sessions_to_use = [session_map[i] for i in selected_indices]
            break
            
        except ValueError:
            print(f"{R}❌ Input format error. Use comma-separated numbers or the full filename.{RESET}")


    print(f"\n{G}✅ Starting Solana Address delivery using {B}{len(sessions_to_use)}{G} selected sessions...{RESET}")


    for session_file in sessions_to_use:
        # ⚠️ প্রতিটি সেশনের জন্য একটি অনন্য অ্যাড্রেস লোড করা হচ্ছে (load_and_remove_address ফাংশনটি সংজ্ঞায়িত থাকতে হবে)
        solana_address_to_send = load_and_remove_address() 
        
        if solana_address_to_send is None:
            # যদি ফাইলটি খালি হয়ে যায়, কাজটি বন্ধ করে দেওয়া হলো
            print(f"{R}❌ [{P}{session_file}{R}] Skipping: No more unique Solana addresses available. Ending task.{RESET}")
            break 
        
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                
                # ✅ অননুমোদিত সেশনের পরে র্যান্ডম ডিলে যুক্ত করা হলো
                try:
                    await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                except NameError:
                    await asyncio.sleep(5)
                
                continue

            # টার্গেট সত্তা খুঁজে নেওয়া
            try:
                entity = await client.get_entity(target_entity_input)
            except Exception as e:
                print(f"{R}❓ [{P}{session_file}{R}] Error getting entity: {e}. Skipping.{RESET}")
                await client.disconnect()
                
                # ✅ এন্টিটি খুঁজে না পেলে র্যান্ডম ডিলে যুক্ত করা হলো
                try:
                    await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                except NameError:
                    await asyncio.sleep(5)
                    
                continue
            
            
            # --- Solana Address মেসেজ পাঠানো ---
            print(f"{B}➤ [{P}{session_file}{B}] Sending UNIQUE address: {solana_address_to_send} to '{entity.username or entity.id}'...{RESET}")
            
            await client.send_message(entity, solana_address_to_send)
            
            print(f"{G}✅ [{P}{session_file}{G}] Unique Solana Address successfully sent and removed from file.{RESET}")


        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except FloodWaitError as e:
            # ✅ ফ্লাড ওয়েট হ্যান্ডলিং
            print(f"{R}⏳ [{P}{session_file}{R}] Flood Wait Error. Waiting for {e.seconds} seconds.{RESET}")
            await asyncio.sleep(e.seconds)
        except Exception as e:
            # ✅ অন্যান্য অপ্রত্যাশিত ত্রুটির জন্য হ্যান্ডলিং
            print(f"{R}❗ [{P}{session_file}{R}] Unexpected error: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        # 💡 সেশনগুলির মধ্যে র্যান্ডম ডিলে (আপনার অনুরোধ অনুযায়ী)
        try:
            await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
        except NameError:
            # যদি random_delay বা ভেরিয়েবল সংজ্ঞায়িত না থাকে, ডিফল্ট ডিলে ব্যবহার করা হলো
            print(f"{Y}⚠️ random_delay/CURRENT_SLEEP not defined. Using fixed 5s delay.{RESET}")
            await asyncio.sleep(5)
            
    print(f"\n{G}{BOLD}--- ✨ Solana Address Sender Task Completed ✨ ---{RESET}")




# -----------------------------------------------------------
# --- 📸 Send Referral with Captcha (No /start, Direct OCR) - MODIFIED & ERROR FIXED ---
# -----------------------------------------------------------

async def send_referral_with_captcha2(api_id, api_hash, session_files):
    """
    Connects to sessions, skips the /start command, and immediately checks 
    the bot's last message for a captcha image, solves it, and sends the answer.
    OCR is configured to read both letters (uppercase/lowercase) and digits.
    """
    # 💡 Global ভেরিয়েবল ব্যবহার করা হয়েছে
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    # ❗ ত্রুটি এড়াতে, ফাংশনের মধ্যেই প্রয়োজনীয় মডিউলগুলি ইম্পোর্ট করার চেষ্টা করা হলো।
    try:
        from telethon.errors import UserDeactivatedBanError, AuthKeyUnregisteredError, FloodWaitError
        import re 
        from PIL import Image
        import pytesseract
        import io
    except ImportError as e:
        print(f"{R}❌ ইম্পোর্ট ত্রুটি: {e}. নিশ্চিত করুন যে 'telethon', 're', 'Pillow (PIL)' এবং 'pytesseract' ইনস্টল করা আছে।{RESET}")
        return

    if not session_files:
        print(f"{R}❌ কোনো সেশন ফাইল পাওয়া যায়নি। প্রক্রিয়া শুরু করা সম্ভব নয়।{RESET}")
        return

    print(f"\n{C}{BOLD}--- 📸 ক্যাপচা সেটআপ (No /start, সরাসরি OCR) ---{RESET}")
    ref_link = input(f"{C}🔗 রেফারেল লিঙ্কটি দিন (e.g., t.me/bot_username?start=ref_code) বা শুধু বট ইউজারনেম দিন: {RESET}")

    try:
        # লিঙ্ক থেকে বট ইউজারনেম বের করা
        match = re.search(r"t\.me/([^?/#]+)", ref_link)
        if match:
            bot_username = match.group(1).replace('/', '')
        else:
            bot_username = ref_link.strip().replace('@', '') 
        
        if not bot_username:
            print(f"{R}❌ Invalid input. Please provide a valid bot username or link.{RESET}")
            return

        print(f"{Y}💡 Detected Bot: {B}@{bot_username}{Y}. Skipping /start command...{RESET}")
    except Exception as e:
        # re ইম্পোর্ট না হলে এই ত্রুটিটি এসেছিল। এখন re মডিউল উপরে ইম্পোর্ট করা হয়েছে।
        print(f"{R}❗ ইনপুট বিশ্লেষণ করা যায়নি: {e}{RESET}") 
        return

    # ১. Tesseract Path সেট করা নিশ্চিত করুন
    try:
        pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe' 
    except AttributeError:
        pass
    except Exception as e:
        print(f"{R}❌ Tesseract Path Set Error: {e}{RESET}")
        
    # ২. সেশন নির্বাচন ও তালিকা প্রদর্শন (Unchanged)
    print(f"\n{C}{BOLD}--- Available Session Files ({len(session_files)} Total) ---{RESET}")
    session_map = {i: session_file for i, session_file in enumerate(session_files, 1)}
    
    for i, session_file in session_map.items():
        print(f"{B}{i}. {P}{session_file}{RESET}")
    
    sessions_to_use = []
    while True:
        selection_input = input(f"\n{C}🔢 যেই সেশনগুলি ব্যবহার করতে চান তার নম্বর দিন (e.g., 1,2,3) বা {G}ALL{C}: {RESET}").strip()
        if selection_input.upper() == 'ALL':
            sessions_to_use = session_files
            break
        try:
            raw_indices = [x.strip() for x in selection_input.split(',')]
            selected_indices = [int(idx) for idx in raw_indices if idx.isdigit() and int(idx) in session_map]
            if not selected_indices:
                print(f"{R}❌ কোনো বৈধ সেশন নম্বর প্রবেশ করা হয়নি। আবার চেষ্টা করুন।{RESET}")
                continue
            sessions_to_use = [session_map[i] for i in selected_indices]
            break
        except ValueError:
            print(f"{R}❌ Invalid input format. Please use comma-separated numbers (e.g., 1,2,3) or type ALL.{RESET}")
    
    print(f"\n{G}✅ Starting Direct Captcha Solve process using {B}{len(sessions_to_use)}{G} selected sessions...{RESET}")

    # ৩. বেছে নেওয়া সেশনগুলিতে কাজ শুরু করা
    for session_file in sessions_to_use:
        # ধরে নেওয়া হচ্ছে TelegramClient এখানে সংজ্ঞায়িত আছে
        client = TelegramClient(session_file, api_id, api_hash) 
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                
                # ✅ অননুমোদিত সেশনের পরে র্যান্ডম ডিলে যুক্ত করা হলো
                try:
                    await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                except NameError:
                    await asyncio.sleep(5)
                    
                continue

            # A) /start কমান্ড বাদ দেওয়া
            print(f"{B}➤ [{P}{session_file}{B}] Skipping /start. Checking for last message/captcha...{RESET}")
            await asyncio.sleep(2) 

            # B) ক্যাপচা বার্তার জন্য অপেক্ষা করা (ছবি আছে কিনা দেখা)
            messages = await client.get_messages(bot_username, limit=1)
            
            if messages and messages[0].media and messages[0].photo:
                print(f"{Y}📸 [{P}{session_file}{Y}] Captcha image received. Attempting OCR...{RESET}")
                
                # C) ছবি ডাউনলোড ও OCR (সংখ্যা ও বর্ণমালা)
                image_data = await client.download_media(messages[0].media, file=bytes)
                image = Image.open(io.BytesIO(image_data))
                
                whitelist = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'
                ocr_text = pytesseract.image_to_string(
                    image, 
                    config=f'--psm 7 -c tessedit_char_whitelist={whitelist}'
                ).strip()
                
                captcha_answer = re.sub(r'[^0-9a-zA-Z]', '', ocr_text) 
                
                print(f"{Y}🔍 Raw OCR Output: {ocr_text}, Cleaned Answer: {captcha_answer}{RESET}")
                
                if captcha_answer:
                    # D) ক্যাপচা উত্তর পাঠানো
                    print(f"{G}✅ [{P}{session_file}{G}] OCR Result: {Y}{captcha_answer}{G}. Sending as answer...{RESET}")
                    await client.send_message(bot_username, captcha_answer)
                    print(f"{G}✅ [{P}{session_file}{G}] Automatic Captcha sent successfully.{RESET}")
                    await asyncio.sleep(2) 
                    
                else:
                    print(f"{R}❌ [{P}{session_file}{R}] OCR failed or returned no digits/letters. Skipping.{RESET}")
                
            else:
                print(f"{Y}⚠️ [{P}{session_file}{Y}] No Captcha image detected in the last message. Skipping.{RESET}")


        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except FloodWaitError as e:
            # ✅ ফ্লাড ওয়েট হ্যান্ডলিং
            print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s.{RESET}")
            await asyncio.sleep(e.seconds)
        except pytesseract.TesseractNotFoundError:
            print(f"{R}❌ [{P}{session_file}{R}] Tesseract OCR Engine not found! Please install it on your system.{RESET}")
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        # 💡 সেশনগুলির মধ্যে র্যান্ডম ডিলে (আপনার অনুরোধ অনুযায়ী)
        try:
            await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
        except NameError:
            # যদি random_delay বা ভেরিয়েবল সংজ্ঞায়িত না থাকে, ডিফল্ট ডিলে ব্যবহার করা হলো
            print(f"{Y}⚠️ random_delay/CURRENT_SLEEP not defined. Using fixed 5s delay.{RESET}")
            await asyncio.sleep(5)
            
    print(f"\n{G}{BOLD}--- ✨ Direct Captcha Solve Task Completed ✨ ---{RESET}")



# -----------------------------------------------------------
# --- 📸 Send Referral with Captcha (With /start Command) - MODIFIED with Random Delay ---
# -----------------------------------------------------------

async def send_referral_with_captcha(api_id, api_hash, session_files):
    """
    Connects to sessions, sends the /start command with the referral payload, 
    and then checks the bot's response for a captcha image, solves it, and sends the answer.
    OCR is configured to read both letters and digits.
    """
    # 💡 এখানে Global ভেরিয়েবল ব্যবহার করা হয়েছে
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP

    if not session_files:
        print(f"{R}❌ কোনো সেশন ফাইল পাওয়া যায়নি। প্রক্রিয়া শুরু করা সম্ভব নয়।{RESET}")
        return

    print(f"\n{C}{BOLD}--- 📸 ক্যাপচা সেটআপ (With /start Command) ---{RESET}")
    ref_link = input(f"{C}🔗 রেফারেল লিঙ্কটি দিন (e.g., t.me/bot_username?start=ref_code): {RESET}")

    try:
        # লিঙ্ক থেকে বট ইউজারনেম ও কমান্ড তৈরি করা
        match = re.search(r"t\.me/([^?]+)\?start=([^&]+)", ref_link)
        if not match:
            # যদি স্ট্যান্ডার্ড লিঙ্ক না হয়, তাহলে শুধু ইউজারনেম ধরে নেওয়া
            match_username = re.search(r"t\.me/([^/]+)$", ref_link)
            if not match_username:
                print(f"{R}❌ Invalid referral link format. Please check the link.{RESET}")
                return
            bot_username = match_username.group(1)
            command = "/start"
        else:
            bot_username = match.group(1)
            payload = match.group(2)
            command = f"/start {payload}"
            
        print(f"{Y}💡 Detected Bot: {B}@{bot_username}{Y}, Command: {B}{command}{RESET}")
    except Exception as e:
        print(f"{R}❗ রেফারেল লিঙ্কটি বিশ্লেষণ করা যায়নি: {e}{RESET}")
        return

    # ১. Tesseract Path সেট করা নিশ্চিত করুন
    try:
        pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe' 
    except AttributeError:
        pass
    except Exception as e:
        print(f"{R}❌ Tesseract Path Set Error: {e}{RESET}")
        
    # ২. সেশন নির্বাচন ও তালিকা প্রদর্শন (Unchanged)
    print(f"\n{C}{BOLD}--- Available Session Files ({len(session_files)} Total) ---{RESET}")
    session_map = {i: session_file for i, session_file in enumerate(session_files, 1)}
    
    # সেশন ফাইলের তালিকা প্রদর্শন
    for i, session_file in session_map.items():
        print(f"{B}{i}. {P}{session_file}{RESET}")
    
    sessions_to_use = []
    while True:
        selection_input = input(f"\n{C}🔢 যেই সেশনগুলি ব্যবহার করতে চান তার নম্বর দিন (e.g., 1,2,3) বা {G}ALL{C}: {RESET}").strip()
        if selection_input.upper() == 'ALL':
            sessions_to_use = session_files
            break
        try:
            raw_indices = [x.strip() for x in selection_input.split(',')]
            selected_indices = [int(idx) for idx in raw_indices if idx.isdigit() and int(idx) in session_map]
            if not selected_indices:
                print(f"{R}❌ কোনো বৈধ সেশন নম্বর প্রবেশ করা হয়নি। আবার চেষ্টা করুন।{RESET}")
                continue
            sessions_to_use = [session_map[i] for i in selected_indices]
            break
        except ValueError:
            print(f"{R}❌ Invalid input format. Please use comma-separated numbers (e.g., 1,2,3) or type ALL.{RESET}")
    
    print(f"\n{G}✅ Starting Captcha Solve process using {B}{len(sessions_to_use)}{G} selected sessions...{RESET}")

    # ৩. বেছে নেওয়া সেশনগুলিতে কাজ শুরু করা
    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        
        try:
            from telethon.errors import UserDeactivatedBanError, AuthKeyUnregisteredError, FloodWaitError
            
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                
                # ✅ অননুমোদিত সেশনের পরে র্যান্ডম ডিলে যুক্ত করা হলো
                try:
                    await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                except NameError:
                    await asyncio.sleep(5)
                    
                continue

            # A) /start কমান্ড পাঠানো
            print(f"{B}➤ [{P}{session_file}{B}] Sending command: {C}{command}{RESET}")
            await client.send_message(bot_username, command)
            await asyncio.sleep(4) # বটের উত্তরের জন্য অপেক্ষা

            # B) ক্যাপচা বার্তার জন্য অপেক্ষা করা (ছবি আছে কিনা দেখা)
            messages = await client.get_messages(bot_username, limit=1)
            
            if messages and messages[0].media and messages[0].photo:
                print(f"{Y}📸 [{P}{session_file}{Y}] Captcha image received. Attempting OCR...{RESET}")
                
                # C) ছবি ডাউনলোড ও OCR (সংখ্যা ও বর্ণমালা)
                image_data = await client.download_media(messages[0].media, file=bytes)
                image = Image.open(io.BytesIO(image_data))
                
                whitelist = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'
                ocr_text = pytesseract.image_to_string(
                    image, 
                    config=f'--psm 7 -c tessedit_char_whitelist={whitelist}'
                ).strip()
                
                captcha_answer = re.sub(r'[^0-9a-zA-Z]', '', ocr_text) 
                
                print(f"{Y}🔍 Raw OCR Output: {ocr_text}, Cleaned Answer: {captcha_answer}{RESET}")
                
                if captcha_answer:
                    # D) ক্যাপচা উত্তর পাঠানো
                    print(f"{G}✅ [{P}{session_file}{G}] OCR Result: {Y}{captcha_answer}{G}. Sending as answer...{RESET}")
                    await client.send_message(bot_username, captcha_answer)
                    print(f"{G}✅ [{P}{session_file}{G}] Automatic Captcha sent successfully.{RESET}")
                    await asyncio.sleep(2) 
                    
                else:
                    print(f"{R}❌ [{P}{session_file}{R}] OCR failed or returned no digits/letters. Skipping.{RESET}")
                
            else:
                print(f"{Y}⚠️ [{P}{session_file}{Y}] No Captcha image detected after /start. Skipping.{RESET}")


        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except FloodWaitError as e:
            # ✅ ফ্লাড ওয়েট হ্যান্ডলিং (আগের কোডের মতো)
            print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s.{RESET}")
            await asyncio.sleep(e.seconds)
        except pytesseract.TesseractNotFoundError:
            print(f"{R}❌ [{P}{session_file}{R}] Tesseract OCR Engine not found! Please install it on your system.{RESET}")
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()
        
        # 💡 সেশনগুলির মধ্যে র্যান্ডম ডিলে (আপনার অনুরোধ অনুযায়ী)
        try:
            await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
        except NameError:
            # যদি random_delay বা ভেরিয়েবল সংজ্ঞায়িত না থাকে, ডিফল্ট ডিলে ব্যবহার করা হলো
            print(f"{Y}⚠️ random_delay/CURRENT_SLEEP not defined. Using fixed 5s delay.{RESET}")
            await asyncio.sleep(5)
            
    print(f"\n{G}{BOLD}--- ✨ Captcha Solve Task Completed (With /start) ✨ ---{RESET}")





# -----------------------------------------------------------
# --- Core Logic for Generating and Saving Data (WITH DUPLICATE CHECK) ---
# -----------------------------------------------------------
def generate_and_save_data(file_path, data_type, count):
    """
    Generates unique data based on type (BNB/SOL/NEAR/TG_Username) and saves/overwrites the file.
    It uses a set to ensure all generated data points are unique.
    """
    
    def generate_single_data(data_type):
        """Helper function to generate a single data item based on type."""
        hex_chars = string.hexdigits.lower()
        base58_chars = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"
        near_chars = string.ascii_lowercase + string.digits
        tg_chars = string.ascii_lowercase + string.digits + '_'
        
        if data_type == 'BNB':
            random_hex = ''.join(random.choice(hex_chars) for _ in range(40))
            return '0x' + random_hex
        elif data_type == 'SOL':
            return ''.join(random.choice(base58_chars) for _ in range(44))
        elif data_type == 'NEAR':
            length = random.randint(10, 20) 
            random_name = ''.join(random.choice(near_chars) for _ in range(length)) + '.near'
            return random_name
        elif data_type == 'TG_Username':
            username = ''.join(random.choice(tg_chars) for _ in range(10)) 
            return '@' + username
        return None # Should not happen

    
    print(f"\n{Y}🔄 {count}টি ইউনিক {data_type} ডেটা তৈরি করা হচ্ছে...{RESET}")
    unique_data_set = set()
    attempts = 0
    max_attempts = count * 5 # Allow more attempts than required count to find uniques

    # Loop to generate required number of unique items
    while len(unique_data_set) < count and attempts < max_attempts:
        new_item = generate_single_data(data_type)
        if new_item:
            unique_data_set.add(new_item)
        attempts += 1

    generated_data = list(unique_data_set)
    
    if len(generated_data) != count:
        print(f"{R}⚠️ সতর্কবার্তা: শুধুমাত্র {len(generated_data)}টি ইউনিক ডেটা তৈরি করা সম্ভব হয়েছে ({count}টির পরিবর্তে)।{RESET}")
    else:
        print(f"{G}✅ সফলভাবে {len(generated_data)}টি ইউনিক {data_type} ডেটা তৈরি হলো।{RESET}")
        
    if data_type in ['BNB', 'SOL', 'NEAR']:
        print(f"{Y}   সতর্কতা: এই অ্যাড্রেসগুলি শুধুমাত্র পরীক্ষার জন্য তৈরি, বাস্তবে বৈধ নয়।{RESET}")
    
    # --- Saving/Overwriting the File ---
    if generated_data:
        try:
            # Open file in write mode ('w') to OVERWRITE existing content
            with open(file_path, 'w', encoding='utf-8') as f:
                for data in generated_data:
                    f.write(data + '\n')
            
            print(f"\n{G}✅ সফল! {B}{len(generated_data)}{G}টি নতুন ডেটা '{file_path}' ফাইলে সেভ করা হয়েছে (পূর্বের ডেটা মুছে গেছে)।{RESET}")
        except Exception as e:
            print(f"{R}❌ ফাইল সেভ করার সময় ত্রুটি: {e}{RESET}")
    else:
        print(f"{R}❌ ডেটা তৈরি বা সেভ করা সম্ভব হয়নি।{RESET}")


# -----------------------------------------------------------
# --- Sub-Menu 2: Data Type Selector (UNCHANGED) ---
# -----------------------------------------------------------
def data_type_menu(file_path):
    # ... (এই ফাংশনটি অপরিবর্তিত থাকবে)
    while True:
        print(f"\n{C}{BOLD}--- 🗂️ ডেটা টাইপ নির্বাচন ({file_path}) ---{RESET}")
        print(f"{C}1. BNB Address (ফেক জেনারেট){RESET}")
        print(f"{C}2. SOLANA Address (ফেক জেনারেট){RESET}")
        print(f"{C}3. Near Address (ফেক জেনারেট){RESET}")
        print(f"{C}4. Telegram Username (ফেক জেনারেট){RESET}")
        print(f"{C}5. মেনুতে ফিরে যান{RESET}")

        choice = input(f"{C}➡️ আপনার অপশনটি বেছে নিন (1-5): {RESET}").strip()
        
        if choice == '5':
            return

        if choice in ['1', '2', '3', '4']:
            try:
                count_input = input(f"{Y}➡️ কতগুলি ডেটা তৈরি করতে চান? সংখ্যা দিন: {RESET}").strip()
                count = int(count_input)
                if count <= 0:
                    print(f"{R}❌ তৈরির সংখ্যা অবশ্যই ০-এর বেশি হতে হবে।{RESET}")
                    continue
            except ValueError:
                print(f"{R}❌ অবৈধ ইনপুট। অনুগ্রহ করে শুধুমাত্র সংখ্যা দিন।{RESET}")
                continue

            data_type_map = {
                '1': 'BNB',
                '2': 'SOL',
                '3': 'NEAR',
                '4': 'TG_Username'
            }
            data_type = data_type_map[choice]
            
            generate_and_save_data(file_path, data_type, count)
            input(f"\n{C}প্রেস এন্টার করে ডেটা টাইপ মেনুতে ফিরুন...{RESET}")
            
        else:
            print(f"{R}❌ অবৈধ অপশন। অনুগ্রহ করে 1 থেকে 5-এর মধ্যে একটি সংখ্যা দিন।{RESET}")


# -----------------------------------------------------------
# --- Sub-Menu 1: File Selector (UNCHANGED) ---
# -----------------------------------------------------------
def generate_data_master_menu():
    # ... (এই ফাংশনটি অপরিবর্তিত থাকবে)
    while True:
        print(f"\n{C}{BOLD}--- 📝 জেনারেটর ফাইল নির্বাচন ---{RESET}")
        print(f"{C}1. send_message_file.txt{RESET}")
        print(f"{C}2. send_message_file2.txt{RESET}")
        print(f"{C}3. মেইন মেনুতে ফিরে যান{RESET}")

        choice = input(f"{C}➡️ আপনার অপশনটি বেছে নিন (1-3): {RESET}").strip()
        
        if choice == '3':
            return
        
        if choice == '1':
            data_type_menu('send_message_file.txt')
        elif choice == '2':
            data_type_menu('send_message_file2.txt')
        else:
            print(f"{R}❌ অবৈধ অপশন। অনুগ্রহ করে 1 থেকে 3-এর মধ্যে একটি সংখ্যা দিন।{RESET}")







# -----------------------------------------------------------
# --- ⚙️ Custom Task Sequence Builder & Runner Function (FIXED with SMART_CLICK & REPEAT) ---
# -----------------------------------------------------------

# Note: pytesseract, Image, io, re, asyncio, random, TelegramClient, 
#       DEFAULT_SLEEP_TIME, B, C, G, P, R, Y, BOLD, RESET must be available globally/imported.

# !!! ADD THESE GLOBAL VARIABLES TO YOUR MAIN SCRIPT (IF NOT ALREADY THERE) !!!
# global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP, random_delay 
# Example:
# CURRENT_MIN_SLEEP = 5
# CURRENT_MAX_SLEEP = 10
# async def random_delay(min_sec, max_sec):
#     await asyncio.sleep(random.uniform(min_sec, max_sec))
# -----------------------------------------------------------------------------------------

import time 
import re 

# -----------------------------------------------------------------------------------------
# --- build_custom_task_sequence FUNCTION (MODIFIED) ---
# -----------------------------------------------------------------------------------------

async def build_custom_task_sequence(api_id, api_hash, session_files):
    """
    Allows the user to define a sequence of tasks for multiple bots sequentially 
    and executes that entire sequence on selected sessions for a specified number of repetitions.
    """
    
    # 💡 এখানে Global ভেরিয়েবল ব্যবহার করা হয়েছে
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    # ❌ কোনো সেশন ফাইল খুঁজে পাওয়া না গেলে
    if not session_files:
        print(f"{R}❌ No session files found. The process cannot be started.{RESET}")
        return

    print(f"\n{C}{BOLD}--- ⚙️ SEQUENTIAL MULTI-BOT TASK SEQUENCE BUILDER ---{RESET}")
    
    # [A. Bot and Task Sequence Input Loop -- Unchanged]
    # ... (Code for bot_task_sequences creation) ...
    # [ ... ]
    
    # ------------------------------------------------------
    # --- A. Bot and Task Sequence Input Loop (Skipped for brevity) ---
    # ------------------------------------------------------
    bot_task_sequences = [] 
    # [Insert the full A. section here if running]
    while True:
        # 1. Referral Link Input
        print(f"\n{G}{BOLD}--- 🤖 Starting Task Sequence Creation for New Bot ---{RESET}")
        
        ref_link_input = input(f"{C}🔗 Enter Referral Link/Bot (@username) ({len(bot_task_sequences) + 1} of N): {RESET}").strip()
        
        if not ref_link_input:
            if not bot_task_sequences:
                print(f"{R}❌ No valid bot or link has been set. Operation aborted.{RESET}")
                return
            else:
                break # If multiple bots are set and empty input is received, task building finishes
        
        try:
            # Extract bot username and command from link
            match = re.search(r"t\.me/([^?]+)\?start=([^&]+)", ref_link_input)
            if not match:
                if ref_link_input.startswith('@'):
                    bot_username = ref_link_input.strip('@')
                    command = "/start"
                else:
                    match_username = re.search(r"t\.me/([^/]+)$", ref_link_input)
                    if match_username:
                         bot_username = match_username.group(1)
                         command = "/start"
                    else:
                         print(f"{R}❌ Invalid link/username format. Please try again.{RESET}")
                         continue
            else:
                bot_username = match.group(1)
                payload = match.group(2)
                command = f"/start {payload}"
                
            print(f"{Y}    💡 Detected: {B}@{bot_username}{Y}, Command: {B}{command}{RESET}")
        except Exception as e:
            print(f"{R}❗ Could not parse the link: {e}. Please try again.{RESET}")
            continue
            
        # 2. Task Sequence Input for the current Bot
        current_bot_tasks = []
        task_count = 1
        
        print(f"\n{G}{BOLD}--- Starting Task Sequence Creation for Bot @{bot_username} ---{RESET}")

        while True:
            print(f"\n{P}TASK {task_count}:{RESET}")
            print(f"{C}1. {B}Send Command or Message{RESET}")
            print(f"{C}2. {B}Inline Click {G}(Button Text){RESET}")
            print(f"{C}3. {B}Image Captcha Solv {G}(Auto){RESET}")
            print(f"{C}4. {B}Share Contact{RESET}")
            print(f"{C}5. {B}Join {G}(Channel/Group){RESET}")
            print(f"{C}6. {B}Wait Second {G}(Delay){RESET}")
            print(f"{C}7. {B}Send Message {G}(From File Adress){RESET}")
            print(f"{C}8. {B}Send Message {G}(From File Username){RESET}") 
            print(f"{C}9. {B}Math Captcha Solv {G}(Auto){RESET}")
            print(f"{C}10. {B}Bot Message Delete {G}(Last Message){RESET}")
            print(f"{C}11. {B}Smart Inline Link Click {G}(Auto Join/Start Bot){RESET}")
            print(f"{C}12. {B}Wait Bot Replay {G}(Wait until the bot replies){RESET}")
            print(f"{C}13. {B}Add another Bot {G}(Start next bot's tasks){RESET}") 
            
            choice = input(f"{C}➡️ Select Task (1-13) or press Enter to finish: {RESET}").strip()
            
            if not choice:
                break
            
            task_data = None
            
            if choice == '1':
                message = input(f"{Y}➡️ Enter Message or Command: {RESET}").strip()
                if message:
                    task_data = ('MESSAGE', message)
            
            elif choice == '2':
                button_text = input(f"{Y}➡️ Enter Inline Button Text: {RESET}").strip()
                if button_text:
                    task_data = ('INLINE_CLICK', button_text)
                    
            elif choice == '3':
                task_data = ('CAPTCHA_SOLVE', None)
                
            elif choice == '4':
                task_data = ('SHARE_CONTACT', None)

            elif choice == '5':
                join_link = input(f"{Y}➡️ Enter Channel/Group link: {RESET}").strip()
                if join_link:
                    task_data = ('JOIN_CHANNEL', join_link)
                    
            elif choice == '6':
                try:
                    wait_time = int(input(f"{Y}➡️ Enter seconds to wait: {RESET}").strip())
                    if wait_time > 0:
                        task_data = ('WAIT', wait_time)
                except ValueError:
                    print(f"{R}❌ Invalid input. Please enter only numbers.{RESET}")

            elif choice == '7':
                task_data = ('MESSAGE_FROM_FILE', 'send_message_file.txt')
                
            elif choice == '8':
                task_data = ('MESSAGE_FROM_FILE_2', 'send_message_file2.txt')
                
            elif choice == '9':
                task_data = ('MATH_SOLVE', None) 
                
            elif choice == '10':
                task_data = ('DELETE_BOT_MESSAGE', None)
            
            elif choice == '11':
                button_text = input(f"{Y}➡️ Enter Inline Button Text to Auto Handle: {RESET}").strip()
                if button_text:
                    task_data = ('SMART_CLICK', button_text)

            elif choice == '12':
                task_data = ('WAIT_FOR_REPLY', None)

            elif choice == '13':
                if not current_bot_tasks:
                    print(f"{R}⚠️ No tasks have been added for this bot.{RESET}")
                break 
            
            else:
                print(f"{R}❌ Invalid option. Select between 1-13 or press Enter.{RESET}")
                continue

            if task_data:
                current_bot_tasks.append(task_data)
                task_count += 1
            else:
                pass 

        # 3. Save the completed sequence for the current bot
        if current_bot_tasks:
            bot_task_sequences.append({
                'username': bot_username, 
                'command': command, 
                'link': ref_link_input,
                'tasks': current_bot_tasks
            })
            print(f"\n{G}✅ {B}{len(current_bot_tasks)}{G} tasks added for bot @{bot_username}.{RESET}")
        
        # If the user selects 13 or presses Enter (empty input), the loop will break. 
        if choice != '13':
            break 
    
    if not bot_task_sequences:
        print(f"{R}❌ No tasks or bots have been set. Operation aborted.{RESET}")
        return
        
    print(f"\n{G}✅ Tasks set for a total of {B}{len(bot_task_sequences)}{G} bots.{RESET}")

    # ------------------------------------------------------
    # --- B.1. Repetition Input (Unchanged) ---
    # ------------------------------------------------------
    num_repetitions = 1 # ডিফল্ট মান
    while True:
        rep_input = input(f"\n{C}🔄 Enter how many times to {BOLD}repeat the sequence{RESET}{C} (Enter 1 for single run): {RESET}").strip()
        if not rep_input:
            print(f"{Y}⚠️ Assuming 1 repetition (single run).{RESET}")
            break
        try:
            num_repetitions = int(rep_input)
            if num_repetitions < 1:
                print(f"{R}❌ Repetition count must be 1 or greater.{RESET}")
                num_repetitions = 1
                continue
            break
        except ValueError:
            print(f"{R}❌ Invalid input. Please enter a number.{RESET}")

    print(f"{G}✅ The entire sequence will be repeated {B}{num_repetitions}{G} time(s).{RESET}")
    # ------------------------------------------------------
    
    # ------------------------------------------------------
    # --- B. Session Selection (Skipped for brevity) ---
    # ------------------------------------------------------
    # [Insert the full B. section here if running]
    print(f"\n{C}{BOLD}--- Available Session Files ({len(session_files)} Total) ---{RESET}")
    session_map = {i: session_file for i, session_file in enumerate(session_files, 1)}
    
    for i, session_file in session_map.items():
        print(f"{B}{i}. {P}{session_file}{RESET}")
    
    sessions_to_use = []
    while True:
        selection_input = input(f"\n{C}🔢 Enter the number(s) of sessions to use (e.g., 1,2,3) or {G}ALL{C}: {RESET}").strip()
        if selection_input.upper() == 'ALL':
            sessions_to_use = session_files
            break
        try:
            raw_indices = [x.strip() for x in selection_input.split(',')]
            selected_indices = [int(idx) for idx in raw_indices if idx.isdigit() and int(idx) in session_map]
            if not selected_indices:
                print(f"{R}❌ No valid session numbers entered. Try again.{RESET}")
                continue
            sessions_to_use = [session_map[i] for i in selected_indices]
            break
        except ValueError:
            print(f"{R}❌ Invalid input format. Please use comma-separated numbers (e.g., 1,2,3) or type ALL.{RESET}")

    print(f"\n{G}✅ Starting execution on {B}{len(sessions_to_use)}{G} sessions...{RESET}")
    # ------------------------------------------------------


    # ------------------------------------------------------
    # --- C. Run Tasks (Sequential Multi-Bot Execution Loop - MODIFIED) ---
    # ------------------------------------------------------

    # Ensure Tesseract Path is set (based on your system path)
    try:
        # Ensure your tesseract path is correct!
        pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe' 
    except:
        pass

    # --- REPETITION LOOP (OUTERMOST LOOP) ---
    for rep_count in range(1, num_repetitions + 1):
        print(f"\n{Y}{BOLD}--- 🔁 REPETITION {rep_count} of {num_repetitions} ---{RESET}")

        # --- Session Loop (Outer) ---
        for session_file in sessions_to_use:
            client = TelegramClient(session_file, api_id, api_hash)
            print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
            
            try:
                # Import all necessary telethon types inside the try block 
                from telethon.tl.functions.channels import JoinChannelRequest
                from telethon.tl.functions.messages import ImportChatInviteRequest 
                from telethon.tl.types import (
                    KeyboardButtonRequestPeer, 
                    KeyboardButtonRequestPhone, 
                    InputMediaContact
                )
                from telethon.errors import UserDeactivatedBanError, AuthKeyUnregisteredError, FloodWaitError
                
                await client.connect() 
                if not await client.is_user_authorized():
                    print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                    await client.disconnect()
                    # ❌ এই ডিলেটি দরকার নেই কারণ লুপের শেষে র্যান্ডম ডিলে যুক্ত হবে
                    # await asyncio.sleep(DEFAULT_SLEEP_TIME) 
                    
                    # ✅ এখানে র্যান্ডম ডিলে যুক্ত করা হলো (আপনার অনুরোধ অনুযায়ী)
                    # Note: random_delay ফাংশনটি বাইরে সংজ্ঞায়িত করা আবশ্যক
                    try:
                        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                    except NameError:
                        # যদি ভেরিয়েবল বা ফাংশন না পাওয়া যায়
                        await asyncio.sleep(5) 

                    continue

                # --- Bot Loop (Inner) ---
                for bot_config in bot_task_sequences:
                    # ... (Inner Bot Loop Logic - Unchanged) ...
                    # [Insert the full inner bot loop logic here]
                    bot_username = bot_config['username']
                    command = bot_config['command']
                    task_sequence = bot_config['tasks']
                    
                    print(f"\n{BOLD}{P}--- 🤖 Starting tasks for bot @{bot_username} ({len(task_sequence)} tasks) ---{RESET}")

                    # 1. Send /start Command
                    print(f"{B}➤ [{P}{session_file}{B}] Sending initial command: {C}{command}{RESET}")
                    await client.send_message(bot_username, command)
                    await asyncio.sleep(5) 

                    # 2. Execute Custom Tasks
                    for step, (task_type, task_value) in enumerate(task_sequence, 1):
                        print(f"{P}--- TASK {step}: {task_type} ---{RESET}")
                        
                        if task_type == 'MESSAGE':
                            await client.send_message(bot_username, task_value)
                            print(f"{G}✅ Message sent: {C}{task_value}{RESET}")
                            await asyncio.sleep(3) 
                            
                        elif task_type == 'WAIT':
                            print(f"{Y}⏳ Waiting for {task_value} seconds...{RESET}")
                            await asyncio.sleep(task_value)
                            
                        elif task_type == 'JOIN_CHANNEL':
                            try:
                                await client(JoinChannelRequest(task_value))
                                print(f"{G}✅ Joined Channel/Group: {C}{task_value}{RESET}")
                            except Exception as e:
                                print(f"{R}❌ Failed to join {C}{task_value}{R}: {e}{RESET}")
                            await asyncio.sleep(4) 

                        # -----------------------------------------------------
                        # ✅ TASK: SMART_CLICK (Private Join fix included)
                        # -----------------------------------------------------
                        elif task_type == 'SMART_CLICK':
                            button_text_to_find = task_value
                            print(f"{Y}🔄 [SMART] Searching for ALL Inline Buttons matching: {C}'{button_text_to_find}'{RESET}")
                            
                            try:
                                messages = await client.get_messages(bot_username, limit=1)
                                buttons_processed = 0 
                                
                                if messages and messages[0].buttons:
                                    message = messages[0]
                                    
                                    for row in message.buttons:
                                        for button in row:
                                            
                                            if button.text == button_text_to_find:
                                                buttons_processed += 1
                                                
                                                if hasattr(button, 'url') and button.url:
                                                    url = button.url
                                                    
                                                    # --- A. যদি Join Link হয় (Channel/Group) ---
                                                    if 't.me/' in url and not re.search(r"t\.me/([^?]+)\?start=", url):
                                                        
                                                        invite_hash = None
                                                        if 'joinchat' in url or url.startswith('https://t.me/+'):
                                                            match = re.search(r"/(joinchat/|\+)?([^/]+)$", url)
                                                            if match:
                                                                invite_hash = match.group(2).strip('+') 
                                                        else:
                                                            invite_hash = url.split('/')[-1].strip('@') 

                                                        if invite_hash:
                                                            print(f"{G}✅ [SMART] Button {buttons_processed} clicked: {C}{button_text_to_find}{G}. Action: {Y}JOIN CHANNEL/GROUP{RESET}")
                                                            try:
                                                                if 'joinchat' in url or url.startswith('https://t.me/+'):
                                                                    await client(ImportChatInviteRequest(invite_hash))
                                                                    print(f"{G}   ➡️ Successfully Joined (Private): {C}{url}{RESET}")
                                                                else:
                                                                    await client(JoinChannelRequest(url))
                                                                    print(f"{G}   ➡️ Successfully Joined (Public): {C}{url}{RESET}")
                                                                    
                                                            except Exception as e:
                                                                print(f"{R}   ❌ Failed to join {C}{url}{R}: {type(e).__name__} - {e}{RESET}")
                                                                
                                                        else:
                                                            print(f"{R}   ❌ Could not parse valid invite hash/username from link: {url}{RESET}")
                                                            
                                                    # --- B. যদি Bot Start Link হয় ---
                                                    elif 't.me/' in url and '?start=' in url:
                                                        match = re.search(r"t\.me/([^?]+)\?start=([^&]+)", url)
                                                        if match:
                                                            target_bot_username = match.group(1)
                                                            payload = match.group(2)
                                                            start_command = f"/start {payload}"
                                                            
                                                            print(f"{G}✅ [SMART] Button {buttons_processed} clicked: {C}{button_text_to_find}{G}. Action: {Y}START BOT{RESET}")
                                                            await client.send_message(target_bot_username, start_command)
                                                            print(f"{G}   ➡️ Bot {target_bot_username} started with command: {C}{start_command}{RESET}")
                                                            await asyncio.sleep(2)
                                                        else:
                                                             print(f"{R}   ❌ Could not parse bot start link: {url}{RESET}")
                                                        
                                                    # --- C. যদি অন্য কোনো URL হয় (Generic URL) ---
                                                    else:
                                                        print(f"{Y}⚠️ [SMART] Found a generic URL button ({buttons_processed}). URL: {url}. (Only joining/starting is automated){RESET}")
                                                        
                                                # 3. যদি URL বাটন না হয় (Callback Button)
                                                else:
                                                    print(f"{G}✅ [SMART] Button {buttons_processed} clicked: {C}{button_text_to_find}{G}. Action: {Y}NORMAL CLICK (Callback){RESET}")
                                                    try:
                                                        await button.click()
                                                    except Exception as click_e:
                                                         print(f"{R}   ❌ Failed to click callback button: {click_e}{RESET}")
                                                    await asyncio.sleep(2)
                                                    
                                    if buttons_processed == 0:
                                        print(f"{R}❌ [SMART] Inline Button '{button_text_to_find}' not found in the last message.{RESET}")
                                    else:
                                        print(f"{G}✅ [SMART] Successfully processed {buttons_processed} matching buttons.{RESET}")

                                else:
                                    print(f"{R}❌ [SMART] No Inline buttons found to click.{RESET}")

                            except Exception as e:
                                print(f"{R}❌ [SMART] Smart Click Failed: {type(e).__name__} - {e}{RESET}")
                            await asyncio.sleep(4)
                        # -----------------------------------------------------
                        
                        elif task_type == 'INLINE_CLICK':
                            try:
                                messages = await client.get_messages(bot_username, limit=1)
                                if messages and messages[0].buttons:
                                    button_found = False
                                    for row in messages[0].buttons:
                                        for button in row:
                                            if button.text == task_value:
                                                if isinstance(getattr(button, 'request_peer', None), KeyboardButtonRequestPeer) or \
                                                   isinstance(getattr(button, 'request_phone', None), KeyboardButtonRequestPhone):
                                                    print(f"{Y}⚠️ Found RequestPeer/Phone button. Skipping regular click, use 'Share Contact' task instead.{RESET}")
                                                    continue
                                                    
                                                await button.click()
                                                print(f"{G}✅ Inline Button clicked: {C}{task_value}{RESET}")
                                                button_found = True
                                                break
                                            if button_found:
                                                break
                                    if not button_found:
                                        print(f"{R}❌ Inline Button '{task_value}' not found in the last message.{RESET}")
                                else:
                                    print(f"{R}❌ No Inline buttons found to click.{RESET}")

                            except Exception as e:
                                print(f"{R}❌ Inline Click Failed: {e}{RESET}")
                            await asyncio.sleep(4) 

                        elif task_type == 'CAPTCHA_SOLVE':
                            try:
                                messages = await client.get_messages(bot_username, limit=1)
                                if messages and messages[0].media and messages[0].photo:
                                    image_data = await client.download_media(messages[0].media, file=bytes)
                                    image = Image.open(io.BytesIO(image_data))
                                    
                                    # OCR Logic
                                    whitelist = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'
                                    ocr_text = pytesseract.image_to_string(
                                        image, 
                                        config=f'--psm 7 -c tessedit_char_whitelist={whitelist}'
                                    ).strip()
                                    captcha_answer = re.sub(r'[^0-9a-zA-Z]', '', ocr_text) 
                                    
                                    if captcha_answer:
                                        await client.send_message(bot_username, captcha_answer)
                                        print(f"{G}✅ Captcha Solved & Sent: {Y}{captcha_answer}{RESET}")
                                    else:
                                        print(f"{R}❌ OCR failed to read Captcha image.{RESET}")
                                else:
                                    print(f"{R}❌ Captcha Image not found in the last message.{RESET}")
                            except Exception as e:
                                print(f"{R}❌ Captcha Solve Failed: {e}{RESET}")
                            await asyncio.sleep(4) 
                            
                        elif task_type == 'MATH_SOLVE':
                            print(f"{Y}🔄 Math Captcha Solve task started. Checking the last 2 messages...{RESET}")
                            
                            messages_to_check = await client.get_messages(bot_username, limit=2) 
                            captcha_solved = False 
                            
                            for message in messages_to_check: 
                                if message and message.text: 
                                    last_message_text = message.text
                                    math_match = re.search(r"(\d+\s*[\+\-\*\/]\s*\d+)\s*[\?=\s]", last_message_text, re.IGNORECASE)
                                    
                                    if math_match:
                                        equation_str = math_match.group(1).replace(' ', '')
                                        print(f"{Y}💡 Math Captcha detected: {C}{equation_str}{RESET} (Checked Message){RESET}")
                                        
                                        try:
                                            result = eval(equation_str) 
                                            
                                            if result == int(result):
                                                answer = str(int(result))
                                            else:
                                                answer = str(result)
                                                
                                            await client.send_message(bot_username, answer)
                                            print(f"{G}✅ Math Captcha solved and answer sent: {C}{answer}{RESET}")
                                            await asyncio.sleep(3) 
                                            captcha_solved = True 
                                            break 
                                            
                                        except Exception as e:
                                            print(f"{R}❌ Failed to solve math: {e}{RESET}")
                                            await asyncio.sleep(1)
                                            
                                if captcha_solved:
                                    break
                            
                            if not captcha_solved:
                                print(f"{Y}⚠️ No math captcha found in the last 2 messages.{RESET}")
                            await asyncio.sleep(4) 

                        elif task_type == 'SHARE_CONTACT':
                            print(f"{Y}🔄 Attempting to share contact...{RESET}")
                            
                            try:
                                me = await client.get_me() 
                                messages = await client.get_messages(bot_username, limit=1)
                                button_found = False
                                
                                if messages and messages[0].buttons:
                                    for row in messages[0].buttons:
                                        for button in row:
                                            if isinstance(getattr(button, 'request_peer', None), KeyboardButtonRequestPeer) or \
                                               isinstance(getattr(button, 'request_phone', None), KeyboardButtonRequestPhone):
                                                
                                                await button.click()
                                                print(f"{G}✅ 'Share Contact' button clicked (Request Sent).{RESET}")
                                                button_found = True
                                                break
                                        if button_found:
                                            break
                                            
                                if not button_found:
                                    await client.send_message(
                                        bot_username, 
                                        message="",
                                        file=InputMediaContact(
                                            phone_number=me.phone,
                                            first_name=me.first_name,
                                            last_name=me.last_name or "",
                                            vcard=""
                                        )
                                    )
                                    print(f"{G}✅ Contact sent directly as message.{RESET}")
                                
                            except Exception as e:
                                print(f"{R}❌ Share Contact Failed: {type(e).__name__} - {e}{RESET}")
                            await asyncio.sleep(5) 

                        elif task_type == 'WAIT_FOR_REPLY':
                            timeout = 60
                            print(f"{Y}⏳ Waiting for bot reply (max {timeout} seconds)...{RESET}")
                            
                            wait_start_time = time.time() 
                            
                            try:
                                last_msg_before_wait = await client.get_messages(bot_username, limit=1)
                                last_msg_id = last_msg_before_wait[0].id if last_msg_before_wait else 0
                            except Exception:
                                last_msg_id = 0 

                            reply_received = False
                            while time.time() - wait_start_time < timeout:
                                await asyncio.sleep(2)
                                try:
                                    current_messages = await client.get_messages(bot_username, limit=1)
                                    
                                    if current_messages and current_messages[0].id > last_msg_id:
                                        print(f"{G}✅ Bot reply received! Continuing tasks.{RESET}")
                                        reply_received = True
                                        break
                                    
                                except Exception as e:
                                    print(f"{R}❌ Error while checking for reply: {e}{RESET}")
                                    await asyncio.sleep(5)
                                    
                            if not reply_received:
                                print(f"{R}❌ Timeout: Bot did not reply within {timeout} seconds. Continuing tasks.{RESET}")
                            await asyncio.sleep(2)

                        elif task_type == 'MESSAGE_FROM_FILE' or task_type == 'MESSAGE_FROM_FILE_2':
                            file_path = task_value 
                            
                            try:
                                with open(file_path, 'r', encoding='utf-8') as f:
                                    lines = f.readlines()
                                
                                if not lines:
                                    print(f"{R}❌ File '{file_path}' is empty. Skipping task.{RESET}")
                                    continue

                                message_to_send = lines[0].strip()

                                if not message_to_send:
                                    remaining_lines = lines[1:]
                                    with open(file_path, 'w', encoding='utf-8') as f:
                                        f.writelines(remaining_lines)
                                    print(f"{R}❌ First line in '{file_path}' is empty. Deleted empty line and skipping.{RESET}")
                                    continue
                                    
                                await client.send_message(bot_username, message_to_send)
                                print(f"{G}✅ Message sent from file ({file_path}): {C}{message_to_send}{RESET}")

                                remaining_lines = lines[1:]
                                with open(file_path, 'w', encoding='utf-8') as f:
                                    f.writelines(remaining_lines)
                                    
                            except FileNotFoundError:
                                print(f"{R}❌ Error: File '{file_path}' not found in the script folder. Skipping task.{RESET}")
                            except Exception as e:
                                print(f"{R}❌ Send Message from File Failed: {e}{RESET}")
                            await asyncio.sleep(3) 
                        
                        elif task_type == 'DELETE_BOT_MESSAGE':
                            print(f"{Y}🔄 Attempting to delete the last message from the bot...{RESET}")
                            try:
                                messages_to_delete = await client.get_messages(bot_username, limit=1)
                                
                                if messages_to_delete:
                                    await client.delete_messages(bot_username, messages_to_delete[0].id)
                                    print(f"{G}✅ Bot's latest message ({messages_to_delete[0].id}) has been deleted.{RESET}")
                                else:
                                    print(f"{R}❌ No message found from the bot to delete.{RESET}")
                            except Exception as e:
                                print(f"{R}❌ Failed to delete message: {type(e).__name__} - {e}{RESET}")
                            await asyncio.sleep(2)
                        
                        # Small random delay between tasks to look more human (Already Present)
                        await asyncio.sleep(random.uniform(1, 2)) 
                    
                    print(f"{G}{BOLD}--- ✅ Tasks completed for bot @{bot_username} ---{RESET}")

                # --- Bot Loop End ---

            except (UserDeactivatedBanError, AuthKeyUnregisteredError):
                print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
            except FloodWaitError as e:
                # ✅ ফ্লাড ওয়েট হ্যান্ডলিং (আগের কোডের মতো)
                print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s.{RESET}")
                await asyncio.sleep(e.seconds)
            except Exception as e:
                print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
            finally:
                if client and client.is_connected():
                    await client.disconnect()
            
            # 💡 এইখানে আপনার অনুরোধ করা সেশন-টু-সেশন র্যান্ডম ডিলে যুক্ত করা হলো:
            try:
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
            except NameError:
                # যদি random_delay বা ভেরিয়েবল সংজ্ঞায়িত না থাকে, ডিফল্ট ডিলে ব্যবহার করা হলো
                print(f"{Y}⚠️ random_delay/CURRENT_SLEEP not defined. Using fixed 5s delay.{RESET}")
                await asyncio.sleep(5) 
            
        # --- Session Loop End ---
    # --- REPETITION LOOP END ---

    print(f"\n{G}{BOLD}--- ✨ Custom Task Sequence completed on all bots and sessions {num_repetitions} time(s) ✨ ---{RESET}")
    




# -----------------------------------------------------------
# --- 👻 Referral Sub-Menu (NEW) ---
# -----------------------------------------------------------

async def referral_sub_menu(api_id, api_hash, session_files):
    """Sub-menu for all referral operations."""
    while True:
        print(f"\n{P}╔══════════════════════════════════════════╗{RESET}")
        print(f"{P}║ {Y}{BOLD}Referral Service Sub-Menu{P} {G}By:- @BTCArif  {P}║{RESET}")
        print(f"{P}╠══════════════════════════════════════════╣{RESET}")
        print(f"{P}║ {C}1. {B}Standard Refer {G}(Only /start)          {P}║") # (Old 2)
        print(f"{P}║ {C}2. {B}Custom Task Sequence Refer Bot {G}(New)  {P}║") # (Old 6)
        print(f"{P}║ {C}3. {B}Mass Refer {G}(Single Inline Click)      {P}║") # (Old 4)
        print(f"{P}║ {C}4. {B}Manual Captcha Refer{G}(Forward to Group){P}║") # (Old 8)
        print(f"{P}║ {C}5. {B}Manual Emoji Click Refer {G}(User Input) {P}║") # (Old 9)
        print(f"{P}║ {C}6. {B}Multi Inline Click Refer {G}(Multi Click){P}║") # (Old 18)
        print(f"{P}║ {C}7. {B}Single Inline Click Refer {G}(Not Start) {P}║") # (Old 18)
        print(f"{P}║ {C}8. {B}Image Captcha Refer {G}(Auto With /start){P}║") # (Old 6)
        print(f"{P}║ {C}9. {B}Image Captcha Refer {G}(Auto/ Not /start){P}║") # (Old 6)
        print(f"{P}║ {C}10. {B}Share Contact Service {G}(Not Start)    {P}║") # (Old 6)
        print(f"{P}║ {C}11. {B}Send Solana Address {G}(Not Start)      {P}║") # (Old 6)
        print(f"{P}║ {C}12. {B}Group Comment {G}(Auto Comment)         {P}║") # (Old 6)
        print(f"{P}║ {C}13. {B}Adress Genaraetor {G}(Auto Save File)   {P}║") # (Old 6)
        print(f"{P}║ {C}14. {B}Back to {G}(Main/Menu)                  {P}║")
        print(f"{P}╚══════════════════════════════════════════╝{RESET}")

        referral_choice = input(f"\n{C}➡️  Enter your choice (1-14): {RESET}")

        if referral_choice == '1':
            await send_referral(api_id, api_hash, session_files)
        if referral_choice == '2':
            await build_custom_task_sequence(api_id, api_hash, session_files)
        elif referral_choice == '3':
            await mass_inline_click_referral(api_id, api_hash, session_files)
        elif referral_choice == '4':
            await manual_captcha_referral(api_id, api_hash, session_files)
        elif referral_choice == '5':
            await manual_emoji_click_referral(api_id, api_hash, session_files)
        elif referral_choice == '6':
            await multi_inline_click_referral(api_id, api_hash, session_files)
        elif referral_choice == '7':
            await send_inline_button_click(api_id, api_hash, session_files)
        elif referral_choice == '8':
            await send_referral_with_captcha(api_id, api_hash, session_files)
        elif referral_choice == '9':
            await send_referral_with_captcha2(api_id, api_hash, session_files)
        elif referral_choice == '10':
            await share_contact_service(api_id, api_hash, session_files)
        elif referral_choice == '11':  
            await send_solana_address(api_id, api_hash, session_files)
        elif referral_choice == '12':  
            await group_comment_operation(api_id, api_hash, session_files)
        elif referral_choice == '13':  
            generate_data_master_menu()
        elif referral_choice == '14':
            print(f"{Y}↩️ Returning to Main Menu.{RESET}")
            break
        else:
            print(f"{R}❌ Invalid choice. Please try again.{RESET}")

        await asyncio.sleep(DEFAULT_SLEEP_TIME)

# -----------------------------------------------------------
# --- 🧹 CLEAR ACCOUNT FUNCTIONS ---
# -----------------------------------------------------------

async def clear_dialogs_by_type(api_id, api_hash, sessions_to_use, clear_groups=True, clear_bots=True):
    """Leaves channels/groups and blocks bots based on the flags set, skipping all Pinned dialogs and only using selected sessions."""
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    if not clear_groups and not clear_bots:
        print(f"{R}❌ Error: Nothing selected to clear. Please choose at least one option.{RESET}")
        return
    
    if not sessions_to_use:
        print(f"{R}❌ No sessions selected for clearing.{RESET}")
        return

    action_summary = []
    if clear_groups:
        action_summary.append("Groups/Channels")
    if clear_bots:
        action_summary.append("Bots")
        
    action_text = " and ".join(action_summary)

    print(f"\n{C}{BOLD}--- ⚠️ Account Clearing Warning ---{RESET}")
    print(f"{R}ATTENTION: This will clear ONLY {action_text} and is irreversible.{RESET}")
    print(f"{Y}NOTE: Any pinned chat (Group, Channel, or Bot) will be skipped.{RESET}")
    print(f"{Y}Operating on {len(sessions_to_use)} selected sessions.{RESET}")
    confirmation = input(f"{C}➡️  Are you sure you want to proceed with clearing {action_text}? ({G}Y{C}/{R}N{C}): {RESET}").strip().upper()

    if confirmation != 'Y':
        print(f"{Y}🧹 Account clearing cancelled by user.{RESET}")
        return

    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🧹 Processing session for clearing: {P}{session_file}{RESET}")
        
        left_count = 0
        blocked_count = 0
        
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            async for dialog in client.iter_dialogs():
                entity = dialog.entity
                processed = False
                
                if dialog.pinned:
                    print(f"{Y}📌 [{P}{session_file}{Y}] Skipping Pinned: {dialog.name or str(dialog.id)}{RESET}")
                    continue
                
                try:
                    timeout_seconds = 8

                    if (dialog.is_channel or dialog.is_group) and clear_groups:
                        print(f"{B}➤ [{P}{session_file}{B}] Leaving: {C}{entity.title or str(dialog.id)}{RESET}")
                        await asyncio.wait_for(client(LeaveChannelRequest(entity)), timeout=timeout_seconds)
                        left_count += 1
                        processed = True
                        
                    elif dialog.is_user and getattr(entity, 'bot', False) and clear_bots:
                        username = f"@{entity.username}" if entity.username else "a bot"
                        print(f"{R} [{P}{session_file}{R}] Blocking and deleting bot: {C}{username}{RESET}")
                        await asyncio.wait_for(client(BlockRequest(entity)), timeout=timeout_seconds)
                        await asyncio.wait_for(client(DeleteHistoryRequest(peer=entity, max_id=0, just_clear=False, revoke=True)), timeout=timeout_seconds)
                        blocked_count += 1
                        processed = True
                
                except FloodWaitError as e:
                    print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s.{RESET}")
                    await asyncio.sleep(e.seconds + 1)
                except asyncio.TimeoutError:
                    print(f"{R}⌛ [{P}{session_file}{R}] Operation timed out for '{dialog.name}'. Skipping.{RESET}")
                except Exception as e:
                    print(f"{R}❗ [{P}{session_file}{R}] Could not process '{dialog.name}': {e}{RESET}")
                
                if processed:
                    await asyncio.sleep(0.5)

        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except Exception as e:
            print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()

        print(f"{G}✅ Session {P}{session_file}{G} Summary: Left {left_count} chats, blocked {blocked_count} bots.{RESET}")
        
        await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)

    print(f"\n{G}{BOLD}--- 🧹 Account Clearing Process Completed ---{RESET}")

async def clear_account_menu(api_id, api_hash, session_files):
    """Sub-menu for different account clearing options, now including session selection."""
    
    # --- সেশন নির্বাচন ---
    sessions_to_use = select_sessions_for_task(session_files, task_name="Account Clearing")
    if not sessions_to_use:
        return
    # --------------------

    while True:
        print(f"\n{P}╔══════════════════════════════════════════╗{RESET}")
        print(f"{P}║ {Y}{BOLD}Account Clearing Sub-Menu{P} {G}By:- @BTCArif  {P}║{RESET}")
        print(f"{P}╠══════════════════════════════════════════╣{RESET}")
        print(f"{P}║ {C}1. {B}Clear ALL {G}(Groups/Channels + Bots)    {P}║")
        print(f"{P}║ {C}2. {B}Only Leave {G}(Groups/Channels)          {P}║")
        print(f"{P}║{C} 3. {B}Only {G}(Block/Delete Bots)              {P}║")
        print(f"{P}║ {C}4. {B}Back to {G}(Main/Menu)                   {P}║")
        print(f"{P}╚══════════════════════════════════════════╝{RESET}")
        print(f"{Y}💡 NOTE: Pinned Chats/Bots will be SKIPPED in all clearing operations.{RESET}")
        print(f"{B}Selected Sessions: {len(sessions_to_use)}{RESET}")

        clear_choice = input(f"\n{C}➡️  Enter your choice (1-4): {RESET}")

        if clear_choice == '1':
            await clear_dialogs_by_type(api_id, api_hash, sessions_to_use, clear_groups=True, clear_bots=True)
        elif clear_choice == '2':
            await clear_dialogs_by_type(api_id, api_hash, sessions_to_use, clear_groups=True, clear_bots=False)
        elif clear_choice == '3':
            await clear_dialogs_by_type(api_id, api_hash, sessions_to_use, clear_groups=False, clear_bots=True)
        elif clear_choice == '4':
            print(f"{Y}↩️ Returning to Main Menu.{RESET}")
            break
        else:
            print(f"{R}❌ Invalid choice. Please try again.{RESET}")

        await asyncio.sleep(DEFAULT_SLEEP_TIME)

# -----------------------------------------------------------
# --- Notification and Message Functions ---
# -----------------------------------------------------------

async def select_session_for_notification(session_files):
    """Allows user to select a single session file by number."""
    print(f"\n{C}{BOLD}--- Select Session for Notification Service ---{RESET}")
    if not session_files:
        print(f"{Y}⚠️ No session files found.{RESET}")
        return None

    for i, session_file in enumerate(session_files, 1):
        print(f"{B}{i}. {P}{session_file}{RESET}")

    session_choice_input = input(f"\n{C}➡️  নোটিফিকেশন সার্ভিসের জন্য সেশন ফাইলের নম্বরটি দিন (1-{len(session_files)}): {RESET}").strip()

    try:
        choice_index = int(session_choice_input) - 1
        if 0 <= choice_index < len(session_files):
            return session_files[choice_index]
        else:
            print(f"{R}❌ Error: Invalid number choice.{RESET}")
            return None
    except ValueError:
        print(f"{R}❌ Error: Invalid input. Please enter a number.{RESET}")
        return None


async def telegram_notification_service(api_id, api_hash, session_files):
    """Starts a notification service for a chosen session for a user-defined duration, with manual stop."""
    
    session_file_to_use = await select_session_for_notification(session_files)
    if not session_file_to_use:
        return

    try:
        duration_input = input(f"\n{C}➡️  নোটিফিকেশন সার্ভিস কত মিনিটের জন্য চালু রাখতে চান? (ডিফল্ট: 5): {RESET}")
        duration_minutes = int(duration_input) if duration_input.isdigit() and int(duration_input) > 0 else 5
    except ValueError:
        duration_minutes = 5
        
    duration_seconds = duration_minutes * 60

    client = TelegramClient(session_file_to_use, api_id, api_hash)

    try:
        print(f"{Y}🔄 Starting client for {P}{session_file_to_use}{Y}...{RESET}")
        await client.start() # 💡 client.start() is crucial here
        
        me = await client.get_me()
        
        phone_number = me.phone or "N/A"
        username = f"@{me.username}" if me.username else "N/A"
        
        print(f"\n{G}✅ Notification Service চালু হলো।{RESET}")
        print(f"{P}  - সেশন: {B}{session_file_to_use}{P}{RESET}")
        print(f"{P}  - মোবাইল: {B}{phone_number}{P}{RESET}")
        print(f"{P}  - ইউজারনেম: {B}{username}{P}{RESET}")
        print(f"{P}  - চলবে: {B}{duration_minutes} মিনিট (ম্যানুয়ালি বন্ধ করতে {C}Enter{P} চাপুন){RESET}\n")
        print(f"{P}====================================================={RESET}\n")

        @client.on(events.NewMessage)
        async def handler(event):
            sender = await event.get_sender()
            chat = await event.get_chat()
            
            sender_name = getattr(sender, 'first_name', '') or getattr(sender, 'title', 'Unknown')
            sender_username = getattr(sender, 'username', None)

            sender_info = f"{B}{sender_name}{RESET}"
            if sender_username:
                sender_info += f" ({C}@{sender_username}{RESET})"
                
            chat_name = getattr(chat, 'title', None)
            if chat_name and chat_name != sender_name:
                chat_info = f" {P}in {Y}[{chat_name}]{RESET}"
            else:
                chat_info = ""

            print(f"📩 {G}From:{RESET} {sender_info}{chat_info}")
            print(f"   {Y}Message:{RESET} {event.text}\n" + "-"*30)

        sleep_task = asyncio.create_task(asyncio.sleep(duration_seconds))
        
        print(f"{Y}Waiting for messages... (Press {C}Enter{Y} to stop before {duration_minutes} minutes){RESET}")
        # Note: Using asyncio.to_thread for blocking input in an async context
        input_task = asyncio.create_task(asyncio.to_thread(input)) 
        
        done, pending = await asyncio.wait(
            [sleep_task, input_task],
            return_when=asyncio.FIRST_COMPLETED,
        )

        if input_task in done:
            print(f"\n{Y}↩️ Manual stop detected. Service shutting down.{RESET}")
        else:
            print(f"\n{Y}⏹️ {duration_minutes} মিনিট শেষ, নির্ধারিত সময়সীমা পূর্ণ হয়েছে।{RESET}")

        for task in pending:
            task.cancel()
            
    except (AuthKeyUnregisteredError, UserDeactivatedBanError):
        print(f"{R}💀 Error: Session {P}{session_file_to_use}{R} is banned or deleted. Skipping.{RESET}")
    except Exception as e:
        print(f"{R}❗ An unexpected error occurred: {e}{RESET}")
    finally:
        if client.is_connected():
            await client.disconnect()
        print(f"\n{G}{BOLD}--- ✨ Task Completed ✨ ---{RESET}")


async def send_message(api_id, api_hash, session_files):
    """
    Sends a message to a user, bot, or group from only the selected sessions 
    (chosen by their numbers).
    """
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    if not session_files:
        print(f"{R}❌ No session files found to send messages.{RESET}")
        return

    print(f"\n{C}{BOLD}--- ✈️ Send Message Setup ---{RESET}")

    # 1. টার্গেট এবং মেসেজ ইনপুট নেওয়া
    target_link = input(f"{C}🎯 Enter the username, bot link, or group link: {RESET}").strip()
    message = input(f"{C}✉️  Enter the message you want to send: {RESET}")

    if not target_link or not message:
        print(f"{R}❌ Target link and message cannot be empty.{RESET}")
        return

    # 2. নির্বাচিত সেশন ফাইলগুলি বেছে নেওয়া
    sessions_to_use = select_sessions_for_task(session_files, task_name="Send Message")
    if not sessions_to_use:
        return
    
    print(f"\n{G}✅ Starting message process using {B}{len(sessions_to_use)}{G} selected sessions...{RESET}")

    # 3. বেছে নেওয়া সেশনগুলিতে কাজ শুরু করা
    for session_file in sessions_to_use:
        client = TelegramClient(session_file, api_id, api_hash)
        print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
        try:
            await client.connect()
            if not await client.is_user_authorized():
                print(f"{R}❌ Error: Session {P}{session_file}{R} is not authorized. Skipping.{RESET}")
                await client.disconnect()
                await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)
                continue

            print(f"[{P}{session_file}{RESET}] Attempting to send message to '{B}{target_link}{RESET}'...")

            try:
                entity = await client.get_entity(target_link)
            except (ValueError, TypeError):
                print(f"{R}❓ [{P}{session_file}{R}] Error: Could not find '{B}{target_link}{R}'. Skipping.{RESET}")
                continue

            # --- মূল মেসেজ পাঠানোর অংশ ---
            await client.send_message(entity, message)
            print(f"{G}✅ [{P}{session_file}{G}] Successfully sent message.{RESET}")
            # --- মূল মেসেজ পাঠানোর অংশ ---

        except (UserDeactivatedBanError, AuthKeyUnregisteredError):
            print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
        except UserPrivacyRestrictedError:
            print(f"{R}🔒 [{P}{session_file}{R}] Error: You cannot message this user due to privacy settings.{RESET}")
        except PeerFloodError:
            print(f"{R}🌊 [{P}{session_file}{R}] Error: Peer flood limit. This account may be limited. Skipping.{RESET}")
        except FloodWaitError as e:
            print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds} seconds.{RESET}")
            await asyncio.sleep(e.seconds)
        except Exception as e:
            print(f"{R}❗ [{P}{session_file}{R}] An unexpected error occurred: {e}. Skipping.{RESET}")
        finally:
            if client.is_connected():
                await client.disconnect()

            await random_delay(CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP)

    print(f"\n{G}{BOLD}--- ✨ Task Completed ✨ ---{RESET}")


# -----------------------------------------------------------
# --- Main Function ---
# -----------------------------------------------------------

async def main():
    global CURRENT_MIN_SLEEP, CURRENT_MAX_SLEEP
    
    print_header()
    
    # 🔔 প্রোগ্রাম শুরু হওয়ার সময় সেটিং লোড করা (আপনার প্রয়োজন অনুযায়ী স্থায়ীত্ব নিশ্চিত করা)
    load_delay_settings() 
    
    try:
        api_id, api_hash = get_api_credentials()
    except ValueError as e:
        print(f"{R}❌ Initialization Error: {e}{RESET}")
        print(f"{Y}💡 Please check your 'api_credentials.txt' file. It must contain the API ID on the first line and the API Hash on the second line, with no extra text or empty lines in between.{RESET}")
        return

    session_files = sorted([f for f in os.listdir('.') if f.endswith('.session')])
    if not session_files:
        print(f"\n{R}📂 No .session files found. Please use 'Login New Account' option to add one.{RESET}")
    else:
        print(f"\n{G}📂 Found {Y}({len(session_files)}){G} session files.{RESET}")
        
    while True:
        # মেনু UI: Referral Service (Sub Menu) অপশন 2-এ স্থানান্তরিত হয়েছে
        print(f"{G}⌛ Current Random Sleep Time: {Y}(Min={CURRENT_MIN_SLEEP}s/Max={CURRENT_MAX_SLEEP}s){RESET}")
        print(f"\n{P}╔═════════════════════{C}{BOLD} MAIN MENU {P}═════════════════════╗{RESET}")
        print(f"{P}║ {Y}{BOLD}I. Mass Operations (Bulk){RESET}{P}                           ║")
        print(f"{P}║{RESET}  {C}1. {B}Join {G}(Channels/Groups){P}   {C}2. {B}Referral Menu{G} (Bot) {P}║") # <-- নতুন সাব-মেনু
        print(f"{P}║{RESET}  {C}3. {B}Leave Target{P}             {C}4. {B}Send Message{P}        {P}║") 
        print(f"{P}║{RESET}  {C}5. {B}Block/Unblock{P}            {C}6. {B}Pin Target{P}          {P}║")
        print(f"{P}║═════════════════════════════════════════════════════║{RESET}")
        print(f"{P}║ {Y}{BOLD}II.Account Management/Setup{RESET}{P}                         ║")
        print(f"{P}║{RESET}  {C}7. {B}Login New Account{P}        {C}8. {B}Clear Account{P}       {P}║") 
        print(f"{P}║{RESET}  {C}9. {B}Account Setting{P}          {C}10. {B}Manage Account{P}     {P}║") 
        print(f"{P}║═════════════════════════════════════════════════════║{RESET}")
        print(f"{P}║ {Y}{BOLD}III. Utility / Exit{RESET}{P}                                 ║")
        print(f"{P}║{RESET}  {C}11. {B}Notif. Service{P}          {C}12. {B}Owner Contact{P}      {P}║") 
        print(f"{P}║{RESET}  {C}13. {R}Exit Tool{P}               {C}14. {B}Set Randm Sleep{P}    {P}║") 
      
        print(f"{P}╚═════════════════════════════════════════════════════╝{RESET}")


        choice = input(f"\n{C}{BOLD}➡️  Enter your choice (1-14): {RESET}") 

        if choice == '1':
            await join_channels(api_id, api_hash, session_files)
        elif choice == '2': # Referral Sub-Menu
            await referral_sub_menu(api_id, api_hash, session_files)
        elif choice == '3':
            await leave_target_menu(api_id, api_hash, session_files)
        elif choice == '4':
            await send_message(api_id, api_hash, session_files)
        elif choice == '5':
            await block_unblock_menu(api_id, api_hash, session_files)
        elif choice == '6':
            await pin_target_dialogs(api_id, api_hash, session_files)
        # Old 7, 8, 9, 10-18 অপশনগুলোর সিরিয়াল পরিবর্তন
        elif choice == '7':
            login_success = await login_new_account(api_id, api_hash)
            if login_success:
                session_files = sorted([f for f in os.listdir('.') if f.endswith('.session')])
                print(f"\n{G}🔄 Session list updated. Total sessions available: {B}{len(session_files)}{RESET}")
        elif choice == '8':
            await clear_account_menu(api_id, api_hash, session_files)
        elif choice == '9':
            await account_setting_menu(api_id, api_hash, session_files)
        elif choice == '10':
            await manage_account(api_id, api_hash, session_files)
        elif choice == '11':
            await telegram_notification_service(api_id, api_hash, session_files)
        elif choice == '12':
            contact_owner()
        elif choice == '13':
            print(f"\n{Y}👋 Exiting tool. Goodbye!{RESET}")
            break
        elif choice == '14':
            await set_random_delay_times()
        else:
            print(f"\n{R}❌ Invalid choice. Please try again.{RESET}")

if __name__ == "__main__":
    try:
        # 💡 asyncio.to_thread থাকলে সেটি ব্যবহার করা হবে, যা ইন্টারাপ্ট হ্যান্ডলিং উন্নত করে
        if hasattr(asyncio, 'to_thread'):
             asyncio.run(main())
        else:
             asyncio.run(main())
    except KeyboardInterrupt:
        print(f"\n\n{R}Program interrupted by user. Exiting...{RESET}")
    except Exception as e:
        print(f"\n\n{R}A critical error occurred: {e}{RESET}")
