## **How do `sendto()` and `recvfrom()` differ from `send()` and `recv()`?**

<details>
	<summary>Reveal answer</summary>

- **send** and **recv** are called after calling **connect**, They only work with the one remote address that was passed to `connect()`. <br>
**sendto** and **recvfrom** functions can be used with multiple remote addresses.
</details>

## **Can `send()` and `recv()` be used on UPD sockets ?**

<details>
  <summary>Reveal answer</summary>

  - Oh yeah, but `connect()` function should be called first in that case. 
</details>


## **What does `connect()` do in case of UDP server ?**

<details>
	<summary>Reveal answer</summary>

 - `connect()` associates the socket with a remote given address
</details>

## **What makes multiplexing with UDP easier than with TCP ?**

<details>
	<summary>Reveal answer</summary>

- One UDP socket can talk to multiple remote peers. For TCP, one socket is needed for each peer

</details>

## **Downsides of UDP compared to TCP ?**

<details>
	<summary>Reveal answer</summary>

- UDP does not attempt to fix many of the errors that TCP does... FOr example, TCP ensures that data **arrives on the same order it was sent**, TCP tries to avoid causing network congestion, and TCP attempts to **resend** lost packets. UDP does **none of this**

</details>

## **Can the same program use UDP and TCP?**

<details>
	<summary>Reveal answer</summary>

- Yeah sure, It just needs to create sockets for both.

</details>



