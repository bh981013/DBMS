select t.name, count(c.id)
from Trainer t, CatchedPokemon c
where t.id = c.owner_id
group by t.id
having count(c.id) >= 3
order by count(c.id) desc;