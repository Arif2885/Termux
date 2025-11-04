# -----------------------------------------------------------
# --- ⚙️ Custom Task Sequence Builder & Runner Function (FIXED with SMART_CLICK & REPEAT) ---
# -----------------------------------------------------------

# Note: pytesseract, Image, io, re, asyncio, random, TelegramClient, 
#       DEFAULT_SLEEP_TIME, B, C, G, P, R, Y, BOLD, RESET must be available globally/imported.
# ✅ এই ফাংশনটি ব্যবহার করার জন্য আপনার স্ক্রিপ্টের শুরুতে import time যোগ করতে ভুলতে না।

import time 
import re # <<< Smart Click এর জন্য re (regex) ইম্পোর্ট নিশ্চিত করুন

async def build_custom_task_sequence(api_id, api_hash, session_files):
    """
    Allows the user to define a sequence of tasks for multiple bots sequentially 
    and executes that entire sequence on selected sessions for a specified number of repetitions.
    
    FIXES APPLIED: 
    1. Added new task option 'SMART_CLICK' (Option 11) for Auto Join/Start Bot.
    2. Shifted 'Wait Bot Replay' to Option 12.
    3. Shifted 'Add another Bot' to Option 13.
    4. Added Repetition Loop option (New Section B.1).
    """
    
    # ❌ কোনো সেশন ফাইল খুঁজে পাওয়া না গেলে
    if not session_files:
        print(f"{R}❌ No session files found. The process cannot be started.{RESET}")
        return

    print(f"\n{C}{BOLD}--- ⚙️ SEQUENTIAL MULTI-BOT TASK SEQUENCE BUILDER ---{RESET}")
    
    # এটি সমস্ত বটের জন্য টাস্ক সিকোয়েন্স সংরক্ষণ করবে
    bot_task_sequences = [] 
    
    # ------------------------------------------------------
    # --- A. Bot and Task Sequence Input Loop (UNMODIFIED) ---
    # ------------------------------------------------------
    
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
            # ✅ NEW OPTION 11
            print(f"{C}11. {B}Smart Inline Link Click {G}(Auto Join/Start Bot){RESET}")
            # ✅ SHIFTED TO 12
            print(f"{C}12. {B}Wait Bot Replay {G}(Wait until the bot replies){RESET}")
            # ✅ SHIFTED TO 13
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
            
            # ✅ NEW OPTION 11
            elif choice == '11':
                button_text = input(f"{Y}➡️ Enter Inline Button Text to Auto Handle: {RESET}").strip()
                if button_text:
                    task_data = ('SMART_CLICK', button_text)

            # ✅ SHIFTED OPTION 12 (was 11)
            elif choice == '12':
                task_data = ('WAIT_FOR_REPLY', None)

            # ✅ SHIFTED OPTION 13 (was 12)
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
    # --- B.1. Repetition Input (NEW SECTION) ---
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


    # ------------------------------------------------------
    # --- B. Session Selection (Unchanged) ---
    # ------------------------------------------------------
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
    # --- C. Run Tasks (Sequential Multi-Bot Execution Loop) ---
    # ------------------------------------------------------

    # Ensure Tesseract Path is set (based on your system path)
    try:
        # Ensure your tesseract path is correct!
        pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe' 
    except:
        pass

    # --- REPETITION LOOP (OUTERMOST LOOP) --- (NEW)
    for rep_count in range(1, num_repetitions + 1):
        print(f"\n{Y}{BOLD}--- 🔁 REPETITION {rep_count} of {num_repetitions} ---{RESET}")

        # --- Session Loop (Outer) ---
        for session_file in sessions_to_use:
            client = TelegramClient(session_file, api_id, api_hash)
            print(f"\n{Y}🔄 Processing session: {P}{session_file}{RESET}")
            
            try:
                # Import all necessary telethon types inside the try block 
                from telethon.tl.functions.channels import JoinChannelRequest
                from telethon.tl.functions.messages import ImportChatInviteRequest # For private join fix
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
                    await asyncio.sleep(DEFAULT_SLEEP_TIME)
                    continue

                # --- Bot Loop (Inner) ---
                for bot_config in bot_task_sequences:
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
                        
                        # Small random delay between tasks to look more human
                        await asyncio.sleep(random.uniform(1, 2)) 
                    
                    print(f"{G}{BOLD}--- ✅ Tasks completed for bot @{bot_username} ---{RESET}")

                # --- Bot Loop End ---

            except (UserDeactivatedBanError, AuthKeyUnregisteredError):
                print(f"{R}💀 Error: Session {P}{session_file}{R} is banned or deleted. Skipping.{RESET}")
            except FloodWaitError as e:
                print(f"{R}⏳ [{P}{session_file}{R}] Flood wait error. Waiting for {e.seconds}s.{RESET}")
                await asyncio.sleep(e.seconds)
            except Exception as e:
                print(f"{R}❗ An unexpected error occurred with session {P}{session_file}{R}: {e}. Skipping.{RESET}")
            finally:
                if client and client.is_connected():
                    await client.disconnect()
            await asyncio.sleep(DEFAULT_SLEEP_TIME)
            
        # --- Session Loop End ---
    # --- REPETITION LOOP END ---

    print(f"\n{G}{BOLD}--- ✨ Custom Task Sequence completed on all bots and sessions {num_repetitions} time(s) ✨ ---{RESET}")
