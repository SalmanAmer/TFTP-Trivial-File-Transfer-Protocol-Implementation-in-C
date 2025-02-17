# TFTP-Trivial-File-Transfer-Protocol-Implementation-in-C
This project provides a simple, efficient implementation of the TFTP (Trivial File Transfer Protocol) in C, designed to facilitate file transfers over UDP. The implementation includes both client and server functionalities that support basic operations like file reading (RRQ) and writing (WRQ).

Key Features:
	•	UDP-based communication: Uses UDP sockets to implement TFTP, ensuring fast, lightweight file transfers.
 
	•	Error Handling: Implements basic error handling for common TFTP operations such as file read/write failures.
 
	•	Block-based Data Transfer: Data is transferred in blocks (512 bytes), with proper acknowledgment for reliability.
 
	•	File Deletion (DEL): Supports file deletion through a DEL request from the client.
