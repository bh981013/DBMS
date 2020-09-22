select p.name
from Pokemon p, CatchedPokemon c
where p.id = c.pid and c.nickname like'% %'
order by p.name desc;