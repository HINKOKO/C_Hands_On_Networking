## **Make a program that sends Email**

**SMTP** for **S**imple **M**ail **T**ransfer **P**rotocol, is **the** protocol responsible for delivering email on the Internet.<br>
It is a text-based protocol that operates on **port 25**, a typical session look as follow:

- Client establishes a connection to SMTP server
- Server initiates with a greeting. This greeting indicates the server is ready to receive commands
- Client issues its own greeting
- Server responds
- Client sends a command indicating who the mail is from
- Server responds to indicate sender is accepted
- Client issues another comman, specifying mail's recipient
- Server indicates recipient accepted
- Client issues a **DATA** command
- Server responds asling the client to proceed
- Client transfers the email !

Each client commands start with a four-letter word:

- **HELO** client identify itself to the server
- **MAIL** used to specify **who** is sending the mail
- **RCPT** used to speicfy the recipient
- **DATA** used to initiate the transfer of the actual email. Should include both headers and a body
- **QUIT** used to end the session

The server uses some codes for successful email

- **220** Service is ready
- **250** Requested command was accepted and completed successfully
- **354** Start sending the message
- **221** Closing connection

In this chapter, we build a simple SMTP client able to send short emails.
